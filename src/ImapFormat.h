#pragma once
#include "InputModel.h"
#include <QStringList>

namespace imap
{

    struct LoadResult
    {
        bool ok = false;
        QString error;
        QStringList warnings;
        InputMap map;
    };

    LoadResult parseImap(const QString& text);
    QString writeImap(const InputMap& map, const QString& eol = QStringLiteral("\n"));

} // namespace imap
