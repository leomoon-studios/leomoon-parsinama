#include "services/PrintService.h"

#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

class PrintTests final : public QObject
{
    Q_OBJECT

private slots:
    void multiPagePdf();
    void invalidOutputPath();
};

void PrintTests::multiPagePdf()
{
    PrintContent content;
    content.poet = QStringLiteral("حافظ شیرازی");
    content.collection = QStringLiteral("غزلیات");
    content.title = QStringLiteral("غزل شمارهٔ ۱");
    content.rows.append({QStringLiteral("section"), false, {}, {}, QStringLiteral("بند آغاز")});
    for (int index = 0; index < 220; ++index) {
        content.rows.append({QStringLiteral("verse"), true,
            index == 0 ? QStringLiteral("آغاز یکتا") : QStringLiteral("مصرع نخست فارسی"),
            index == 219 ? QStringLiteral("پایان یکتا") : QStringLiteral("مصرع دوم فارسی"),
            {}});
    }
    const QString html = PrintService::documentHtml(content);
    QVERIFY(html.indexOf(QStringLiteral("بند آغاز")) < html.indexOf(QStringLiteral("آغاز یکتا")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("poem.pdf"));
    QString error;
    QVERIFY2(PrintService::renderPdf(content, path,
        QStringLiteral(PARSINAMA_TEST_FONT_PATH), &error), qPrintable(error));
    QFile pdf(path);
    QVERIFY(pdf.open(QIODevice::ReadOnly));
    QVERIFY(pdf.read(8).startsWith("%PDF-"));
    QVERIFY(pdf.size() > 10000);

    if (!QStandardPaths::findExecutable(QStringLiteral("pdfinfo")).isEmpty()) {
        QProcess info;
        info.start(QStringLiteral("pdfinfo"), {path});
        QVERIFY(info.waitForFinished(10000));
        QCOMPARE(info.exitCode(), 0);
        const QByteArray output = info.readAllStandardOutput();
        const QRegularExpression pages(QStringLiteral("Pages:\\s+(\\d+)"));
        const auto match = pages.match(QString::fromUtf8(output));
        QVERIFY(match.hasMatch());
        QVERIFY(match.captured(1).toInt() > 1);
    }
    if (!QStandardPaths::findExecutable(QStringLiteral("pdffonts")).isEmpty()) {
        QProcess fonts;
        fonts.start(QStringLiteral("pdffonts"), {path});
        QVERIFY(fonts.waitForFinished(10000));
        QCOMPARE(fonts.exitCode(), 0);
        QVERIFY(fonts.readAllStandardOutput().contains("Vazirmatn"));
    }
    const QString pdftotext = QStandardPaths::findExecutable(QStringLiteral("pdftotext"));
    if (!pdftotext.isEmpty()) {
        QProcess help;
        help.start(pdftotext, {QStringLiteral("-h")});
        QVERIFY(help.waitForFinished(10000));
        const QByteArray options = help.readAllStandardError() + help.readAllStandardOutput();
        if (options.contains("-bbox")) {
            QProcess bounds;
            bounds.start(pdftotext, {QStringLiteral("-bbox"), path, QStringLiteral("-")});
            QVERIFY(bounds.waitForFinished(10000));
            const QByteArray boundsError = bounds.readAllStandardError();
            QVERIFY2(bounds.exitCode() == 0, boundsError.constData());
            const QString boxes = QString::fromUtf8(bounds.readAllStandardOutput());
            const auto topWord = QRegularExpression(QStringLiteral("<word[^>]*yMin=\"([0-9.]+)\""))
                .match(boxes);
            QVERIFY(topWord.hasMatch());
            QVERIFY2(topWord.captured(1).toDouble() < 80, "The printed title starts too far down the page");
        }

        QProcess extract;
        extract.start(pdftotext, {path, QStringLiteral("-")});
        QVERIFY(extract.waitForFinished(10000));
        const QByteArray extractError = extract.readAllStandardError();
        QVERIFY2(extract.exitCode() == 0, extractError.constData());
        QString text = QString::fromUtf8(extract.readAllStandardOutput())
            .normalized(QString::NormalizationForm_KC);
        text.remove(QRegularExpression(QStringLiteral("[\\s\\x{200e}\\x{200f}\\x{202a}-\\x{202e}\\x{2066}-\\x{2069}]")));
        QVERIFY(text.contains(QStringLiteral("حافظ")));
        const auto first = text.indexOf(QStringLiteral("آغازیک"));
        const auto last = text.indexOf(QStringLiteral("پایانیک"));
        QVERIFY(first >= 0);
        QVERIFY(last > first);
    }
}

void PrintTests::invalidOutputPath()
{
    PrintContent content;
    content.title = QStringLiteral("شعر");
    content.rows.append({QStringLiteral("verse"), false, {}, {}, QStringLiteral("متن")});
    QString error;
    QVERIFY(!PrintService::renderPdf(content, {}, QStringLiteral(PARSINAMA_TEST_FONT_PATH), &error));
    QVERIFY(!error.isEmpty());
}

QTEST_MAIN(PrintTests)

#include "print_tests.moc"
