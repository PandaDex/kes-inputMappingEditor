#pragma once
#include <QString>
#include <QVector>
#include <vector>

namespace imap
{

    enum class ValueType
    {
        Boolean,
        Vector2,
        Vector3
    };
    enum class ModifierType
    {
        Deadzone,
        Negate,
        Unknown
    };

    QString valueTypeEngineName(ValueType t);
    QString valueTypeDisplayName(ValueType t);
    ValueType valueTypeFromEngineName(const QString& s, bool* known = nullptr);

    // unknown field, written back as-is
    struct RawField
    {
        QString name;
        QString raw;
        bool operator==(const RawField& o) const { return name == o.name && raw == o.raw; }
    };

    struct Modifier
    {
        ModifierType type = ModifierType::Deadzone;
        QString unknownName;
        QString unknownRawValue;
        double scalar = 0.2;
        bool negateX = true;
        bool negateY = true;
        bool negateZ = true;
        QVector<RawField> extra;

        bool operator==(const Modifier& o) const;
        bool operator!=(const Modifier& o) const { return !(*this == o); }
    };

    struct Binding
    {
        QString key;
        QVector<Modifier> modifiers;
        bool playerAssignable = false;
        QString assignableKey = QStringLiteral("none");
        QVector<RawField> extra;

        bool operator==(const Binding& o) const;
        bool operator!=(const Binding& o) const { return !(*this == o); }
    };

    struct Action
    {
        QString name;
        ValueType valueType = ValueType::Boolean;
        QVector<Binding> bindings;
        QVector<RawField> extra;

        bool operator==(const Action& o) const;
        bool operator!=(const Action& o) const { return !(*this == o); }
    };

    struct InputMap
    {
        QVector<Action> actions;
        bool crlf = true;

        bool operator==(const InputMap& o) const { return actions == o.actions; }
    };

    struct ModifierInfo
    {
        ModifierType type;
        const char* engineName;
        const char* displayName;
        const char* tooltip;
    };
    const std::vector<ModifierInfo>& modifierRegistry();
    const ModifierInfo* modifierInfo(ModifierType t);
    ModifierType modifierTypeFromName(const QString& engineName);
    Modifier makeDefaultModifier(ModifierType t);
    QString modifierSummary(const Modifier& m);
    QString formatScalar(double v);

} // namespace imap
