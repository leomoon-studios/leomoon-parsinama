#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QVector>

struct SectionRecord
{
    int index = 0;
    int number = 0;
    QString sectionType;
    QString verseType;
    QString plainText;
    QString poemFormat;
};

struct VerseRecord
{
    int order = 0;
    QString position;
    QString text;
    int coupletIndex = 0;
    int sectionIndex1 = 0;
    int sectionIndex2 = 0;
    QString coupletSummary;
};

struct ReadingRowRecord
{
    bool paired = false;
    QString rightText;
    QString leftText;
    QString text;
    QString position;
};

class SectionListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        IndexRole = Qt::UserRole + 1,
        NumberRole,
        SectionTypeRole,
        VerseTypeRole,
        PlainTextRole,
        PoemFormatRole
    };

    explicit SectionListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const { return m_sections.size(); }
    void replace(QVector<SectionRecord> sections);

signals:
    void countChanged();

private:
    QVector<SectionRecord> m_sections;
};

class VerseListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        OrderRole = Qt::UserRole + 1,
        PositionRole,
        TextRole,
        CoupletIndexRole,
        SectionIndex1Role,
        SectionIndex2Role,
        CoupletSummaryRole
    };

    explicit VerseListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const { return m_verses.size(); }
    void replace(QVector<VerseRecord> verses);

signals:
    void countChanged();

private:
    QVector<VerseRecord> m_verses;
};

class ReadingRowListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        PairedRole = Qt::UserRole + 1,
        RightTextRole,
        LeftTextRole,
        TextRole,
        PositionRole
    };

    explicit ReadingRowListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const { return m_rows.size(); }
    void replace(QVector<ReadingRowRecord> rows);

signals:
    void countChanged();

private:
    QVector<ReadingRowRecord> m_rows;
};

class PoemLoader final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY stateChanged)
    Q_PROPERTY(QString title READ title NOTIFY stateChanged)
    Q_PROPERTY(QString fullUrl READ fullUrl NOTIFY stateChanged)
    Q_PROPERTY(QString metre READ metre NOTIFY stateChanged)
    Q_PROPERTY(QString summary READ summary NOTIFY stateChanged)
    Q_PROPERTY(QAbstractItemModel *sections READ sections CONSTANT)
    Q_PROPERTY(QAbstractItemModel *verses READ verses CONSTANT)
    Q_PROPERTY(QAbstractItemModel *readingRows READ readingRows CONSTANT)

public:
    explicit PoemLoader(QString catalogPath, QObject *parent = nullptr);

    bool loading() const { return m_loading; }
    QString error() const { return m_error; }
    QString statusText() const;
    QString title() const { return m_title; }
    QString fullUrl() const { return m_fullUrl; }
    QString metre() const { return m_metre; }
    QString summary() const { return m_summary; }
    QAbstractItemModel *sections() { return &m_sections; }
    QAbstractItemModel *verses() { return &m_verses; }
    QAbstractItemModel *readingRows() { return &m_readingRows; }

    Q_INVOKABLE void loadByUrl(const QString &url);
    Q_INVOKABLE void clear();

signals:
    void stateChanged();
    void requestFinished(bool success);

private:
    QString m_catalogPath;
    QString m_error;
    QString m_title;
    QString m_fullUrl;
    QString m_metre;
    QString m_summary;
    SectionListModel m_sections;
    VerseListModel m_verses;
    ReadingRowListModel m_readingRows;
    quint64 m_requestSerial = 0;
    bool m_loading = false;
};
