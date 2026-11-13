#pragma once
#include "InputModel.h"
#include <QStringList>
#include <QVector>

namespace imap
{

    enum class Severity
    {
        Info,
        Warning,
        Error
    };

    struct Issue
    {
        Severity severity = Severity::Info;
        QString message;
        int action = -1;
        int binding = -1;
    };

    QVector<Issue> validate(const InputMap& map, const QStringList& loadWarnings = {});

} // namespace imap
