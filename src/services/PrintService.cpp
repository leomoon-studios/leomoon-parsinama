#include "PrintService.h"

#include "data/NavigationController.h"
#include "data/PoemLoader.h"

#include <QAbstractItemModel>
#include <QAbstractTextDocumentLayout>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QLocale>
#include <QPageSize>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextDocument>

namespace {

QString escaped(const QString &text)
{
    return text.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>"));
}

QString familyForFont(const QString &path, QString *error)
{
    const int id = QFontDatabase::addApplicationFont(path);
    if (id < 0 || QFontDatabase::applicationFontFamilies(id).isEmpty()) {
        if (error) *error = QStringLiteral("قلم وزیرمتن برای چاپ بارگذاری نشد.");
        return {};
    }
    return QFontDatabase::applicationFontFamilies(id).first();
}

bool renderDocument(const PrintContent &content, QPrinter &printer,
                    const QString &fontPath, QString *error)
{
    if (content.title.isEmpty() || content.rows.isEmpty()) {
        if (error) *error = QStringLiteral("متن شعر برای چاپ آماده نیست.");
        return false;
    }
    const QString family = familyForFont(fontPath, error);
    if (family.isEmpty()) return false;

    printer.setDocName(content.title);
    printer.setFontEmbeddingEnabled(true);
    // With fullPage disabled, the painter origin is already at the printable area.
    printer.setFullPage(false);

    QPainter painter;
    if (!painter.begin(&printer)) {
        if (error) *error = QStringLiteral("چاپگر یا فایل پی‌دی‌اف باز نشد.");
        return false;
    }

    const QRect page = printer.pageLayout().paintRectPixels(printer.resolution());
    QFont bodyFont(family);
    bodyFont.setPointSize(11);
    const qreal footerHeight = QFontMetricsF(bodyFont, &printer).height() * 1.8;
    const qreal bodyHeight = page.height() - footerHeight;
    QTextDocument document;
    document.documentLayout()->setPaintDevice(&printer);
    document.setDefaultFont(bodyFont);
    document.setDocumentMargin(0);
    document.setHtml(PrintService::documentHtml(content));
    document.setPageSize(QSizeF(page.width(), bodyHeight));
    const int pages = document.pageCount();
    if (pages < 1) {
        painter.end();
        if (error) *error = QStringLiteral("صفحهٔ چاپی ساخته نشد.");
        return false;
    }

    const QLocale persian(QLocale::Persian);
    for (int index = 0; index < pages; ++index) {
        if (index > 0 && !printer.newPage()) {
            painter.end();
            if (error) *error = QStringLiteral("صفحهٔ بعدی چاپ نشد.");
            return false;
        }
        painter.save();
        painter.setClipRect(QRectF(0, 0, page.width(), bodyHeight));
        painter.translate(0, -index * bodyHeight);
        document.drawContents(&painter, QRectF(0, index * bodyHeight, page.width(), bodyHeight));
        painter.restore();
        painter.setFont(bodyFont);
        painter.setPen(Qt::black);
        painter.drawText(QRectF(0, bodyHeight,
                                page.width(), footerHeight), Qt::AlignCenter,
                         persian.toString(index + 1));
    }
    return painter.end();
}

} // namespace

PrintService::PrintService(NavigationController *navigation, PoemLoader *loader,
                           QString fontPath, QObject *parent)
    : QObject(parent), m_navigation(navigation), m_loader(loader),
      m_fontPath(fontPath.isEmpty()
          ? QStringLiteral(":/qt/qml/LeoMoon/ParsiNama/assets/fonts/Vazirmatn[wght].ttf")
          : std::move(fontPath))
{
    connect(navigation, &NavigationController::stateChanged, this, &PrintService::availabilityChanged);
    connect(loader, &PoemLoader::stateChanged, this, &PrintService::availabilityChanged);
}

PrintService::~PrintService()
{
    delete m_dialog;
    delete m_fileDialog;
}

bool PrintService::available() const
{
    return !m_printDialogActive && !m_fileDialog
        && m_navigation->page() == QLatin1String("poem") && !m_loader->loading()
        && m_loader->error().isEmpty() && !m_loader->title().isEmpty()
        && m_loader->fullUrl() == m_navigation->url();
}

PrintContent PrintService::currentContent() const
{
    PrintContent content;
    content.poet = m_navigation->poetName();
    content.collection = m_navigation->bookName().isEmpty()
        ? m_navigation->breadcrumbs().value(m_navigation->breadcrumbs().size() - 2)
              .toMap().value(QStringLiteral("title")).toString()
        : m_navigation->bookName();
    content.title = m_loader->title();
    QAbstractItemModel *model = m_loader->readingRows();
    content.rows.reserve(model->rowCount());
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i, 0);
        content.rows.append({
            model->data(index, ReadingRowListModel::KindRole).toString(),
            model->data(index, ReadingRowListModel::PairedRole).toBool(),
            model->data(index, ReadingRowListModel::RightTextRole).toString(),
            model->data(index, ReadingRowListModel::LeftTextRole).toString(),
            model->data(index, ReadingRowListModel::TextRole).toString()
        });
    }
    return content;
}

QString PrintService::documentHtml(const PrintContent &content)
{
    QString html = QStringLiteral("<html><body dir='rtl' style='color:#111;background:#fff'>"
        "<h1 align='center' style='font-size:17pt;margin:0 0 8pt'>")
        + escaped(content.title) + QStringLiteral("</h1><p align='center' style='font-size:10pt;margin:0 0 14pt'>")
        + escaped(content.poet);
    if (!content.collection.isEmpty()) html += QStringLiteral(" | ") + escaped(content.collection);
    html += QStringLiteral("</p>");
    for (const PrintRow &row : content.rows) {
        if (row.kind == QLatin1String("section")) {
            html += QStringLiteral("<h2 align='center' style='font-size:12pt;margin:14pt 0 7pt'>")
                + escaped(row.text) + QStringLiteral("</h2>");
        } else if (row.paired) {
            html += QStringLiteral("<table dir='rtl' width='100%' cellspacing='0' cellpadding='5' "
                                   "style='margin:6pt 0'><tr><td width='50%' align='center'>")
                + escaped(row.leftText) + QStringLiteral("</td><td width='50%' align='center'>")
                + escaped(row.rightText) + QStringLiteral("</td></tr></table>");
        } else {
            html += QStringLiteral("<p align='center' style='margin:7pt 0'>")
                + escaped(row.text) + QStringLiteral("</p>");
        }
    }
    return html + QStringLiteral("</body></html>");
}

void PrintService::setError(const QString &error)
{
    if (m_error == error) return;
    m_error = error;
    emit errorChanged();
}

bool PrintService::render(const PrintContent &content, QPrinter &printer)
{
    QString message;
    const bool ok = renderDocument(content, printer, m_fontPath, &message);
    setError(message);
    return ok;
}

void PrintService::printCurrentPoem()
{
    if (!available()) return;
    setError({});
    m_printRequestUrl = m_navigation->url();
    m_printDialogActive = true;
    emit availabilityChanged();
    if (!m_dialog) {
        m_printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
        m_printer->setPageSize(QPageSize(QPageSize::A4));
        m_printer->setPageMargins(QMarginsF(17, 18, 17, 18), QPageLayout::Millimeter);
        m_dialog = new QPrintDialog(m_printer.get());
        m_dialog->setWindowTitle(QStringLiteral("چاپ شعر"));
        m_dialog->setWindowModality(Qt::ApplicationModal);
        connect(m_dialog, &QDialog::finished, this, [this](int result) {
            m_printDialogActive = false;
            if (result == QDialog::Accepted) {
                if (m_navigation->url() == m_printRequestUrl && !m_loader->loading()
                    && m_loader->fullUrl() == m_printRequestUrl) {
                    render(currentContent(), *m_printer);
                } else {
                    setError(QStringLiteral("شعر پیش از چاپ تغییر کرد."));
                }
            }
            m_printRequestUrl.clear();
            emit availabilityChanged();
        });
    }
    m_dialog->open();
}

void PrintService::choosePdfDestination()
{
    if (!available()) return;
    setError({});
    const QString poemUrl = m_navigation->url();
    auto *dialog = new QFileDialog;
    m_fileDialog = dialog;
    dialog->setWindowTitle(QStringLiteral("ذخیرهٔ شعر به صورت پی‌دی‌اف"));
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setNameFilter(QStringLiteral("فایل پی‌دی‌اف (*.pdf)"));
    dialog->setDefaultSuffix(QStringLiteral("pdf"));
    dialog->setLabelText(QFileDialog::Accept, QStringLiteral("ذخیره"));
    dialog->setWindowModality(Qt::ApplicationModal);
    QString fileName = m_loader->title();
    fileName.replace(QRegularExpression(QStringLiteral(R"([\\/:*?"<>|])")), QStringLiteral("-"));
    const QString downloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    dialog->selectFile(QDir(downloads.isEmpty() ? QDir::homePath() : downloads)
        .filePath(fileName + QStringLiteral(".pdf")));
    connect(dialog, &QDialog::finished, this, [this, dialog, poemUrl](int result) {
        m_fileDialog = nullptr;
        if (result == QDialog::Accepted && !dialog->selectedFiles().isEmpty()) {
            if (m_navigation->url() == poemUrl && !m_loader->loading()
                && m_loader->fullUrl() == poemUrl) {
                exportCurrentPoemPdf(dialog->selectedFiles().first());
            } else {
                setError(QStringLiteral("شعر پیش از ذخیره تغییر کرد."));
            }
        }
        emit availabilityChanged();
        dialog->deleteLater();
    });
    emit availabilityChanged();
    dialog->open();
}

bool PrintService::exportCurrentPoemPdf(const QString &path)
{
    if (!available()) {
        setError(QStringLiteral("متن شعر برای چاپ آماده نیست."));
        return false;
    }
    QString message;
    const bool ok = renderPdf(currentContent(), path, m_fontPath, &message);
    setError(message);
    return ok;
}

bool PrintService::renderPdf(const PrintContent &content, const QString &path,
                             const QString &fontPath, QString *error)
{
    if (path.isEmpty()) {
        if (error) *error = QStringLiteral("مسیر فایل پی‌دی‌اف مشخص نیست.");
        return false;
    }
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(17, 18, 17, 18), QPageLayout::Millimeter);
    return renderDocument(content, printer, fontPath, error);
}
