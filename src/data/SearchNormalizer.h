#pragma once

#include <QString>

namespace SearchNormalizer {
QString normalize(const QString &text);
QString matchExpression(const QString &query);
}
