#include "PoemLoader.h"

#include <QFileInfo>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QtConcurrent>

#include <utility>

namespace {

struct PoemLoadResult
{
    QString error;
    QString title;
    QString fullUrl;
    QString metre;
    QString summary;
    QVector<SectionRecord> sections;
    QVector<VerseRecord> verses;
};

PoemLoadResult loadPoem(const QString &catalogPath, const QString &url)
{
    PoemLoadResult result;
    if (!QFileInfo(catalogPath).isFile()) {
        result.error = QStringLiteral("پایگاه دادهٔ شعر پیدا نشد.");
        return result;
    }

    QByteArray poemJson;
    const QString connection = QStringLiteral("parsinama-poem-")
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(catalogPath);
        database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        if (!database.open()) {
            result.error = QStringLiteral("پایگاه دادهٔ شعر باز نشد: %1")
                .arg(database.lastError().text());
        } else {
            {
                QSqlQuery query(database);
                if (!query.prepare(QStringLiteral("SELECT data_json FROM poems WHERE full_url = ?"))) {
                    result.error = QStringLiteral("جستجوی شعر آماده نشد: %1")
                        .arg(query.lastError().text());
                } else {
                    query.addBindValue(url);
                    if (!query.exec()) {
                        result.error = QStringLiteral("خواندن شعر ناموفق بود: %1")
                            .arg(query.lastError().text());
                    } else if (!query.next()) {
                        result.error = QStringLiteral("شعر در پایگاه داده پیدا نشد.");
                    } else {
                        poemJson = query.value(0).toByteArray();
                    }
                }
            }
            database.close();
        }
    }
    QSqlDatabase::removeDatabase(connection);
    if (!result.error.isEmpty()) {
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(poemJson, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.error = QStringLiteral("ساختار شعر در پایگاه داده معتبر نیست.");
        return result;
    }
    const QJsonObject poem = document.object();
    result.title = poem.value(QStringLiteral("Title")).toString();
    result.fullUrl = poem.value(QStringLiteral("FullUrl")).toString();
    result.summary = poem.value(QStringLiteral("PoemSummary")).toString();
    result.metre = poem.value(QStringLiteral("Metre")).toObject()
        .value(QStringLiteral("Rhythm")).toString();
    const QJsonArray sections = poem.value(QStringLiteral("Sections")).toArray();
    const QJsonArray verses = poem.value(QStringLiteral("Verses")).toArray();
    result.sections.reserve(sections.size());
    result.verses.reserve(verses.size());
    for (const QJsonValue &value : sections) {
        const QJsonObject section = value.toObject();
        result.sections.append({
            section.value(QStringLiteral("Index")).toInt(),
            section.value(QStringLiteral("Number")).toInt(),
            section.value(QStringLiteral("SectionType")).toString(),
            section.value(QStringLiteral("VerseType")).toString(),
            section.value(QStringLiteral("PlainText")).toString(),
            section.value(QStringLiteral("PoemFormat")).toString()
        });
    }
    for (const QJsonValue &value : verses) {
        const QJsonObject verse = value.toObject();
        result.verses.append({
            verse.value(QStringLiteral("VOrder")).toInt(),
            verse.value(QStringLiteral("Position")).toString(),
            verse.value(QStringLiteral("Text")).toString(),
            verse.value(QStringLiteral("CoupletIndex")).toInt(),
            verse.value(QStringLiteral("SectionIndex1")).toInt(),
            verse.value(QStringLiteral("SectionIndex2")).toInt(),
            verse.value(QStringLiteral("CoupletSummary")).toString()
        });
    }
    if (result.title.isEmpty() || result.fullUrl != url) {
        result.error = QStringLiteral("شناسهٔ شعر در پایگاه داده معتبر نیست.");
    }
    return result;
}

} // namespace

SectionListModel::SectionListModel(QObject *parent) : QAbstractListModel(parent) {}

int SectionListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_sections.size();
}

QVariant SectionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_sections.size()) {
        return {};
    }
    const SectionRecord &section = m_sections.at(index.row());
    switch (role) {
    case IndexRole: return section.index;
    case NumberRole: return section.number;
    case SectionTypeRole: return section.sectionType;
    case VerseTypeRole: return section.verseType;
    case PlainTextRole: return section.plainText;
    case PoemFormatRole: return section.poemFormat;
    default: return {};
    }
}

QHash<int, QByteArray> SectionListModel::roleNames() const
{
    return {{IndexRole, "sectionIndex"}, {NumberRole, "number"},
            {SectionTypeRole, "sectionType"}, {VerseTypeRole, "verseType"},
            {PlainTextRole, "plainText"}, {PoemFormatRole, "poemFormat"}};
}

void SectionListModel::replace(QVector<SectionRecord> sections)
{
    beginResetModel();
    m_sections = std::move(sections);
    endResetModel();
    emit countChanged();
}

VerseListModel::VerseListModel(QObject *parent) : QAbstractListModel(parent) {}

int VerseListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_verses.size();
}

QVariant VerseListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_verses.size()) {
        return {};
    }
    const VerseRecord &verse = m_verses.at(index.row());
    switch (role) {
    case OrderRole: return verse.order;
    case PositionRole: return verse.position;
    case TextRole: return verse.text;
    case CoupletIndexRole: return verse.coupletIndex;
    case SectionIndex1Role: return verse.sectionIndex1;
    case SectionIndex2Role: return verse.sectionIndex2;
    case CoupletSummaryRole: return verse.coupletSummary;
    default: return {};
    }
}

QHash<int, QByteArray> VerseListModel::roleNames() const
{
    return {{OrderRole, "verseOrder"}, {PositionRole, "position"},
            {TextRole, "text"}, {CoupletIndexRole, "coupletIndex"},
            {SectionIndex1Role, "sectionIndex1"}, {SectionIndex2Role, "sectionIndex2"},
            {CoupletSummaryRole, "coupletSummary"}};
}

void VerseListModel::replace(QVector<VerseRecord> verses)
{
    beginResetModel();
    m_verses = std::move(verses);
    endResetModel();
    emit countChanged();
}

PoemLoader::PoemLoader(QString catalogPath, QObject *parent)
    : QObject(parent), m_catalogPath(std::move(catalogPath)),
      m_sections(this), m_verses(this)
{
}

QString PoemLoader::statusText() const
{
    if (m_loading) {
        return QStringLiteral("در حال بارگذاری شعر…");
    }
    return m_error.isEmpty() ? QString() : m_error;
}

void PoemLoader::clear()
{
    ++m_requestSerial;
    m_loading = false;
    m_error.clear();
    m_title.clear();
    m_fullUrl.clear();
    m_metre.clear();
    m_summary.clear();
    m_sections.replace({});
    m_verses.replace({});
    emit stateChanged();
}

void PoemLoader::loadByUrl(const QString &url)
{
    clear();
    if (url.isEmpty()) {
        m_error = QStringLiteral("نشانی شعر مشخص نیست.");
        emit stateChanged();
        emit requestFinished(false);
        return;
    }
    m_loading = true;
    emit stateChanged();
    const quint64 serial = m_requestSerial;
    auto *watcher = new QFutureWatcher<PoemLoadResult>(this);
    connect(watcher, &QFutureWatcher<PoemLoadResult>::finished, this,
            [this, watcher, serial]() {
        PoemLoadResult result = watcher->result();
        watcher->deleteLater();
        if (serial != m_requestSerial) {
            return;
        }
        m_loading = false;
        m_error = std::move(result.error);
        if (m_error.isEmpty()) {
            m_title = std::move(result.title);
            m_fullUrl = std::move(result.fullUrl);
            m_metre = std::move(result.metre);
            m_summary = std::move(result.summary);
            m_sections.replace(std::move(result.sections));
            m_verses.replace(std::move(result.verses));
        }
        emit stateChanged();
        emit requestFinished(m_error.isEmpty());
    });
    watcher->setFuture(QtConcurrent::run([path = m_catalogPath, url]() {
        return loadPoem(path, url);
    }));
}
