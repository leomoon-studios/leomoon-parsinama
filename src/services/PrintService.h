#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

#include <memory>

class NavigationController;
class PoemLoader;
class QFileDialog;
class QPrintDialog;
class QPrinter;

struct PrintRow
{
    QString kind;
    bool paired = false;
    QString rightText;
    QString leftText;
    QString text;
};

struct PrintContent
{
    QString poet;
    QString collection;
    QString title;
    QVector<PrintRow> rows;
};

class PrintService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit PrintService(NavigationController *navigation, PoemLoader *loader,
                          QString fontPath = {}, QObject *parent = nullptr);
    ~PrintService() override;
    bool available() const;
    QString error() const { return m_error; }
    Q_INVOKABLE void printCurrentPoem();
    Q_INVOKABLE void choosePdfDestination();
    Q_INVOKABLE bool exportCurrentPoemPdf(const QString &path);

    static QString documentHtml(const PrintContent &content);
    static bool renderPdf(const PrintContent &content, const QString &path,
                          const QString &fontPath, QString *error = nullptr);

signals:
    void availabilityChanged();
    void errorChanged();

private:
    PrintContent currentContent() const;
    bool render(const PrintContent &content, QPrinter &printer);
    void setError(const QString &error);

    NavigationController *m_navigation;
    PoemLoader *m_loader;
    QString m_fontPath;
    QString m_error;
    std::unique_ptr<QPrinter> m_printer;
    QPointer<QPrintDialog> m_dialog;
    QPointer<QFileDialog> m_fileDialog;
    QString m_printRequestUrl;
    bool m_printDialogActive = false;
};
