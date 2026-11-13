#include "InputModel.h"

namespace imap
{

    QString valueTypeEngineName(ValueType t)
    {
        switch (t)
        {
        case ValueType::Boolean: return QStringLiteral("boolean");
        case ValueType::Vector2: return QStringLiteral("vector2");
        case ValueType::Vector3: return QStringLiteral("vector3");
        }
        return QStringLiteral("vector2");
    }

    QString valueTypeDisplayName(ValueType t)
    {
        switch (t)
        {
        case ValueType::Boolean: return QStringLiteral("Digital (bool)");
        case ValueType::Vector2: return QStringLiteral("Axis2D (Vector2)");
        case ValueType::Vector3: return QStringLiteral("Axis3D (Vector3)");
        }
        return {};
    }

    ValueType valueTypeFromEngineName(const QString& s, bool* known)
    {
        if (known) *known = true;
        if (s == QLatin1String("boolean")) return ValueType::Boolean;
        if (s == QLatin1String("vector3")) return ValueType::Vector3;
        if (s == QLatin1String("vector2")) return ValueType::Vector2;
        // engine silently falls back to vector2
        if (known) *known = false;
        return ValueType::Vector2;
    }

    bool Modifier::operator==(const Modifier& o) const
    {
        if (type != o.type || extra != o.extra) return false;
        switch (type)
        {
        case ModifierType::Deadzone: return qFuzzyCompare(scalar + 1.0, o.scalar + 1.0);
        case ModifierType::Negate:   return negateX == o.negateX && negateY == o.negateY && negateZ == o.negateZ;
        case ModifierType::Unknown:  return unknownName == o.unknownName && unknownRawValue == o.unknownRawValue;
        }
        return false;
    }

    bool Binding::operator==(const Binding& o) const
    {
        return key == o.key && modifiers == o.modifiers && playerAssignable == o.playerAssignable && assignableKey == o.assignableKey && extra == o.extra;
    }

    bool Action::operator==(const Action& o) const
    {
        return name == o.name && valueType == o.valueType && bindings == o.bindings && extra == o.extra;
    }

    const std::vector<ModifierInfo>& modifierRegistry()
    {
        static const std::vector<ModifierInfo> registry = {
            { ModifierType::Deadzone, "InputMod_deadzone", "Dead Zone",
                "Zeroes the input while its 2D length (X, Y) is at or below the threshold. "
                "Larger values pass through unchanged (no rescaling)." },
            {
                ModifierType::Negate,
                "InputMod_negate",
                "Negate",
        }};
        return registry;
    }

    const ModifierInfo* modifierInfo(ModifierType t)
    {
        for (const ModifierInfo& info : modifierRegistry())
            if (info.type == t) return &info;
        return nullptr;
    }

    ModifierType modifierTypeFromName(const QString& engineName)
    {
        for (const ModifierInfo& info : modifierRegistry())
            if (engineName == QLatin1String(info.engineName)) return info.type;
        return ModifierType::Unknown;
    }

    Modifier makeDefaultModifier(ModifierType t)
    {
        Modifier m;
        m.type = t;
        return m;
    }

    QString formatScalar(double v)
    {
        return QString::number(v, 'g', 6);
    }

    QString modifierSummary(const Modifier& m)
    {
        switch (m.type)
        {
        case ModifierType::Deadzone:
            return QStringLiteral("Dead Zone %1").arg(formatScalar(m.scalar));
        case ModifierType::Negate:
        {
            QString axes;
            if (m.negateX) axes += QLatin1Char('X');
            if (m.negateY) axes += QLatin1Char('Y');
            if (m.negateZ) axes += QLatin1Char('Z');
            return axes.isEmpty() ? QStringLiteral("Negate (none)") : QStringLiteral("Negate %1").arg(axes);
        }
        case ModifierType::Unknown:
            return m.unknownName.isEmpty() ? QStringLiteral("Unknown modifier") : m.unknownName;
        }
        return {};
    }

} // namespace imap
