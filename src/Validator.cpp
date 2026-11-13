#include "Validator.h"
#include "KeyCatalog.h"

#include <QHash>
#include <QSet>

namespace imap
{

    namespace
    {
        bool hasModifier(const Binding& b, ModifierType t)
        {
            for (const Modifier& m : b.modifiers)
                if (m.type == t) return true;
            return false;
        }
    }

    QVector<Issue> validate(const InputMap& map, const QStringList& loadWarnings)
    {
        QVector<Issue> out;
        auto add = [&](Severity s, int a, int b, const QString& msg)
        { out.push_back({ s, msg, a, b }); };

        for (const QString& w : loadWarnings)
            add(Severity::Warning, -1, -1, w);

        QHash<QString, int> firstWithName;
        QHash<QString, QSet<int>> actionsUsingKey;

        for (int ai = 0; ai < map.actions.size(); ++ai)
        {
            const Action& a = map.actions[ai];

            if (a.name.trimmed().isEmpty())
                add(Severity::Error, ai, -1, QStringLiteral("Action has no name."));
            else if (a.name.contains(QLatin1Char('"')) || a.name.contains(QLatin1Char('\n')) || a.name.contains(QLatin1Char('\r')))
                add(Severity::Error, ai, -1, QStringLiteral("Action name contains a quote or line break. The engine's reader has no escape sequences, so this would corrupt the file."));
            else if (!a.name.startsWith(QStringLiteral("InputAction_")))
                add(Severity::Info, ai, -1, QStringLiteral("Name doesn't start with \"InputAction_\" like the other actions."));

            if (firstWithName.contains(a.name))
                add(Severity::Error, ai, -1, QStringLiteral("Duplicate action name \"%1\". The engine keeps one state per name, so both entries would be evaluated into the same state.").arg(a.name));
            else
                firstWithName.insert(a.name, ai);

            if (a.bindings.isEmpty())
                add(Severity::Warning, ai, -1, QStringLiteral("Action has no mappings, so it can never trigger."));

            for (int bi = 0; bi < a.bindings.size(); ++bi)
            {
                const Binding& b = a.bindings[bi];
                const KeyInfo* info = KeyCatalog::find(b.key);

                if (b.key.isEmpty())
                    add(Severity::Error, ai, bi, QStringLiteral("No key selected."));
                else if (!info && b.key.startsWith(QStringLiteral("Keyboard_")))
                    add(Severity::Error, ai, bi, QStringLiteral("\"%1\" is not a key the engine can map. It loads without an error but is bound to no scancode, so this mapping never fires. Add the key to KeyboardNameToScancode() in input.cpp first.").arg(b.key));
                else if (!info)
                    add(Severity::Error, ai, bi, QStringLiteral("\"%1\" is not a key the engine can map. InputLoadActionMap() fails here: this action and every action after it will be missing in game.").arg(b.key));
                else
                {
                    actionsUsingKey[b.key].insert(ai);

                    if (a.valueType == ValueType::Boolean && info->axis2D)
                        add(Severity::Warning, ai, bi, QStringLiteral("2D axis on a Digital action: any stick or mouse movement will count as the action being down."));
                }

                for (const Modifier& m : b.modifiers)
                {
                    if (m.type == ModifierType::Unknown)
                        add(Severity::Warning, ai, bi, QStringLiteral("Modifier \"%1\" isn't supported by the engine; it is kept in the file but has no effect.").arg(m.unknownName));
                    else if (m.type == ModifierType::Deadzone && (m.scalar < 0.0 || m.scalar > 1.0))
                        add(Severity::Warning, ai, bi, QStringLiteral("Dead Zone %1 is outside 0..1.").arg(formatScalar(m.scalar)));
                }

                for (int other = 0; other < bi; ++other)
                    if (a.bindings[other].key == b.key && a.bindings[other].modifiers == b.modifiers)
                    {
                        add(Severity::Warning, ai, bi, QStringLiteral("Same key and modifiers as mapping #%1; their values are added together.").arg(other + 1));
                        break;
                    }
            }
        }

        for (int ai = 0; ai < map.actions.size(); ++ai)
            for (int bi = 0; bi < map.actions[ai].bindings.size(); ++bi)
            {
                const QString& key = map.actions[ai].bindings[bi].key;
                const QSet<int> users = actionsUsingKey.value(key);
                if (users.size() > 1)
                {
                    QStringList others;
                    for (int u : users)
                        if (u != ai) others << map.actions[u].name;
                    add(Severity::Info, ai, bi, QStringLiteral("Key is also used by %1.").arg(others.join(QStringLiteral(", "))));
                }
            }

        return out;
    }

} // namespace imap
