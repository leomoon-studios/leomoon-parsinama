#include "SearchNormalizer.h"

#include <QRegularExpression>

namespace SearchNormalizer {

QString normalize(const QString &text)
{
    const QString input = text.normalized(QString::NormalizationForm_KC);
    QString result;
    result.reserve(input.size());
    bool pendingSpace = false;
    for (QChar ch : input) {
        const ushort code = ch.unicode();
        if (ch.category() == QChar::Mark_NonSpacing || ch.category() == QChar::Mark_SpacingCombining
            || code == 0x0640 || code == 0x200c || code == 0x200d || code == 0xfeff) {
            continue;
        }
        if (ch.isSpace()) {
            pendingSpace = !result.isEmpty();
            continue;
        }
        if (pendingSpace) {
            result.append(QLatin1Char(' '));
            pendingSpace = false;
        }
        if (code == 0x064a || code == 0x0649) ch = QChar(0x06cc);
        else if (code == 0x0643) ch = QChar(0x06a9);
        else if (code >= 0x06f0 && code <= 0x06f9) ch = QChar('0' + code - 0x06f0);
        else if (code >= 0x0660 && code <= 0x0669) ch = QChar('0' + code - 0x0660);
        result.append(ch.toLower());
    }
    return result;
}

QString matchExpression(const QString &query)
{
    static const QRegularExpression word(QStringLiteral("[\\p{L}\\p{N}]+"));
    const QString normalized = normalize(query);
    QStringList terms;
    auto iterator = word.globalMatch(normalized);
    while (iterator.hasNext()) {
        terms.append(QLatin1Char('"') + iterator.next().captured() + QStringLiteral("\"*"));
    }
    return terms.join(QStringLiteral(" AND "));
}

} // namespace SearchNormalizer
