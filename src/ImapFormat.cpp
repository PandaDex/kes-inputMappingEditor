#include "ImapFormat.h"

#include <utility>
#include <vector>

namespace imap
{
    namespace
    {

        struct Node
        {
            enum Kind
            {
                String,
                Object,
                Array,
                Bare
            } kind = Bare;
            QString text;
            std::vector<QString> keys;
            std::vector<Node> vals;
            // source span, so unknown fields can be written back verbatim
            int begin = 0;
            int end = 0;
        };

        class Parser
        {
        public:
            explicit Parser(const QString& text) : s_(text), n_(text.size()) {}

            bool parseTop(std::vector<QString>& names, std::vector<Node>& nodes)
            {
                for (;;)
                {
                    skip();
                    if (p_ >= n_) return true;

                    QString name;
                    if (!parseString(name)) return false;
                    skip();
                    if (p_ >= n_ || s_[p_] != QLatin1Char(':'))
                        return fail(QStringLiteral("Expected ':' after \"%1\"").arg(name));
                    ++p_;

                    Node node;
                    if (!parseValue(node)) return false;
                    if (node.kind != Node::Object)
                        return fail(QStringLiteral("Input action \"%1\" must be an object { ... }").arg(name));

                    names.push_back(name);
                    nodes.push_back(std::move(node));
                }
            }

            QString raw(const Node& n) const { return s_.mid(n.begin, n.end - n.begin); }
            const QString& error() const { return error_; }

        private:
            void skip()
            {
                while (p_ < n_ && (s_[p_].isSpace() || s_[p_] == QLatin1Char(',')))
                    ++p_;
            }

            bool fail(const QString& msg)
            {
                int line = 1;
                for (int i = 0; i < p_ && i < n_; ++i)
                    if (s_[i] == QLatin1Char('\n')) ++line;
                error_ = QStringLiteral("Line %1: %2").arg(line).arg(msg);
                return false;
            }

            bool parseString(QString& out)
            {
                skip();
                if (p_ >= n_ || s_[p_] != QLatin1Char('"'))
                    return fail(QStringLiteral("Expected a quoted string"));
                ++p_;
                const int start = p_;
                while (p_ < n_ && s_[p_] != QLatin1Char('"'))
                    ++p_; // engine has no escapes
                if (p_ >= n_) return fail(QStringLiteral("Unterminated string"));
                out = s_.mid(start, p_ - start);
                ++p_;
                return true;
            }

            bool parseValue(Node& node)
            {
                skip();
                node.begin = p_;
                if (p_ >= n_) return fail(QStringLiteral("Unexpected end of file"));

                const QChar c = s_[p_];
                if (c == QLatin1Char('{'))
                {
                    node.kind = Node::Object;
                    ++p_;
                    for (;;)
                    {
                        skip();
                        if (p_ >= n_) return fail(QStringLiteral("Unterminated '{'"));
                        if (s_[p_] == QLatin1Char('}'))
                        {
                            ++p_;
                            break;
                        }

                        QString key;
                        if (!parseString(key)) return false;
                        skip();
                        if (p_ >= n_ || s_[p_] != QLatin1Char(':'))
                            return fail(QStringLiteral("Expected ':' after \"%1\"").arg(key));
                        ++p_;

                        Node v;
                        if (!parseValue(v)) return false;
                        node.keys.push_back(key);
                        node.vals.push_back(std::move(v));
                    }
                }
                else if (c == QLatin1Char('['))
                {
                    node.kind = Node::Array;
                    ++p_;
                    for (;;)
                    {
                        skip();
                        if (p_ >= n_) return fail(QStringLiteral("Unterminated '['"));
                        if (s_[p_] == QLatin1Char(']'))
                        {
                            ++p_;
                            break;
                        }

                        Node v;
                        if (!parseValue(v)) return false;
                        node.vals.push_back(std::move(v));
                    }
                }
                else if (c == QLatin1Char('"'))
                {
                    node.kind = Node::String;
                    if (!parseString(node.text)) return false;
                }
                else
                {
                    node.kind = Node::Bare;
                    const int start = p_;
                    while (p_ < n_ && !s_[p_].isSpace() && s_[p_] != QLatin1Char(',') && s_[p_] != QLatin1Char('}') && s_[p_] != QLatin1Char(']'))
                        ++p_;
                    if (p_ == start) return fail(QStringLiteral("Unexpected character '%1'").arg(c));
                    node.text = s_.mid(start, p_ - start);
                }

                node.end = p_;
                return true;
            }

            const QString& s_;
            int n_;
            int p_ = 0;
            QString error_;
        };

        bool isTrue(const QString& s)
        {
            return s == QLatin1String("true") || s == QLatin1String("1");
        }

        Modifier buildModifier(const Parser& parser, const Node& n)
        {
            Modifier m;
            m.negateX = m.negateY = m.negateZ = false; // engine default when x/y/z missing
            m.scalar = 0.0;

            QString name;
            const Node* value = nullptr;
            for (size_t i = 0; i < n.keys.size(); ++i)
            {
                if (n.keys[i] == QLatin1String("name"))
                    name = n.vals[i].text;
                else if (n.keys[i] == QLatin1String("value"))
                    value = &n.vals[i];
                else
                    m.extra.push_back({ n.keys[i], parser.raw(n.vals[i]) });
            }

            m.type = modifierTypeFromName(name);
            switch (m.type)
            {
            case ModifierType::Deadzone:
                if (value && value->kind != Node::Object) m.scalar = value->text.toDouble();
                break;
            case ModifierType::Negate:
                if (value && value->kind == Node::Object)
                {
                    for (size_t i = 0; i < value->keys.size(); ++i)
                    {
                        const bool v = isTrue(value->vals[i].text);
                        if (value->keys[i] == QLatin1String("x"))
                            m.negateX = v;
                        else if (value->keys[i] == QLatin1String("y"))
                            m.negateY = v;
                        else if (value->keys[i] == QLatin1String("z"))
                            m.negateZ = v;
                    }
                }
                break;
            case ModifierType::Unknown:
                m.unknownName = name;
                if (value) m.unknownRawValue = parser.raw(*value);
                break;
            }
            return m;
        }

        Binding buildBinding(const Parser& parser, const Node& n, const QString& actionName, QStringList& warnings)
        {
            Binding b;
            for (size_t i = 0; i < n.keys.size(); ++i)
            {
                const QString& k = n.keys[i];
                const Node& v = n.vals[i];
                if (k == QLatin1String("key"))
                    b.key = v.text;
                else if (k == QLatin1String("modifiers"))
                {
                    if (v.kind != Node::Array)
                    {
                        warnings << QStringLiteral("%1: \"modifiers\" is not an array; ignored.").arg(actionName);
                        continue;
                    }
                    for (const Node& mn : v.vals)
                    {
                        if (mn.kind != Node::Object)
                        {
                            warnings << QStringLiteral("%1: a modifier entry is not an object; ignored.").arg(actionName);
                            continue;
                        }
                        b.modifiers.push_back(buildModifier(parser, mn));
                    }
                }
                else if (k == QLatin1String("bPlayerAssignable"))
                    b.playerAssignable = isTrue(v.text);
                else if (k == QLatin1String("AssignableKey"))
                    b.assignableKey = v.text;
                else
                    b.extra.push_back({ k, parser.raw(v) });
            }
            return b;
        }

        Action buildAction(const Parser& parser, const QString& name, const Node& n, QStringList& warnings)
        {
            Action a;
            a.name = name;
            for (size_t i = 0; i < n.keys.size(); ++i)
            {
                const QString& k = n.keys[i];
                const Node& v = n.vals[i];
                if (k == QLatin1String("NativeKeys"))
                {
                    if (v.kind != Node::Array)
                    {
                        warnings << QStringLiteral("%1: \"NativeKeys\" is not an array; ignored.").arg(name);
                        continue;
                    }
                    for (const Node& bn : v.vals)
                    {
                        if (bn.kind != Node::Object)
                        {
                            warnings << QStringLiteral("%1: a NativeKeys entry is not an object; ignored.").arg(name);
                            continue;
                        }
                        a.bindings.push_back(buildBinding(parser, bn, name, warnings));
                    }
                }
                else if (k == QLatin1String("ValueType"))
                {
                    bool known = true;
                    a.valueType = valueTypeFromEngineName(v.text, &known);
                    if (!known)
                        warnings << QStringLiteral("%1: unknown ValueType \"%2\"; the engine treats it as vector2.").arg(name, v.text);
                }
                else
                    a.extra.push_back({ k, parser.raw(v) });
            }
            return a;
        }

        // engine reader can't escape, a stray quote or newline would break the whole file
        QString q(QString s)
        {
            s.remove(QLatin1Char('"'));
            s.remove(QLatin1Char('\n'));
            s.remove(QLatin1Char('\r'));
            return QLatin1Char('"') + s + QLatin1Char('"');
        }

        QString boolStr(bool b)
        {
            return q(b ? QStringLiteral("true") : QStringLiteral("false"));
        }

        QString modifierText(const Modifier& m)
        {
            QString out = QStringLiteral("{");
            const ModifierInfo* info = modifierInfo(m.type);
            const QString name = info ? QString::fromLatin1(info->engineName) : m.unknownName;
            out += QStringLiteral("\"name\": ") + q(name);

            switch (m.type)
            {
            case ModifierType::Deadzone:
                out += QStringLiteral(", \"value\": ") + q(formatScalar(m.scalar));
                break;
            case ModifierType::Negate:
                out += QStringLiteral(", \"value\": {\"x\": %1, \"y\": %2, \"z\": %3}")
                           .arg(boolStr(m.negateX), boolStr(m.negateY), boolStr(m.negateZ));
                break;
            case ModifierType::Unknown:
                if (!m.unknownRawValue.isEmpty())
                    out += QStringLiteral(", \"value\": ") + m.unknownRawValue;
                break;
            }

            for (const RawField& f : m.extra)
                out += QStringLiteral(", ") + q(f.name) + QStringLiteral(": ") + f.raw;
            return out + QLatin1Char('}');
        }

        QString bindingText(const Binding& b)
        {
            QStringList mods;
            for (const Modifier& m : b.modifiers)
                mods << modifierText(m);

            QString out = QStringLiteral("{\"key\": %1, \"modifiers\": [%2], \"bPlayerAssignable\": %3, \"AssignableKey\": %4")
                              .arg(q(b.key), mods.join(QStringLiteral(", ")), boolStr(b.playerAssignable), q(b.assignableKey));
            for (const RawField& f : b.extra)
                out += QStringLiteral(", ") + q(f.name) + QStringLiteral(": ") + f.raw;
            return out + QLatin1Char('}');
        }

    } // namespace

    LoadResult parseImap(const QString& text)
    {
        LoadResult result;
        Parser parser(text);

        std::vector<QString> names;
        std::vector<Node> nodes;
        if (!parser.parseTop(names, nodes))
        {
            result.error = parser.error();
            return result;
        }

        for (size_t i = 0; i < names.size(); ++i)
            result.map.actions.push_back(buildAction(parser, names[i], nodes[i], result.warnings));

        if (!text.isEmpty())
            result.map.crlf = text.contains(QStringLiteral("\r\n"));
        result.ok = true;
        return result;
    }

    QString writeImap(const InputMap& map, const QString& eol)
    {
        QString out;
        for (int i = 0; i < map.actions.size(); ++i)
        {
            const Action& a = map.actions[i];
            if (i > 0) out += eol;

            out += q(a.name) + QStringLiteral(": {") + eol;

            if (a.bindings.isEmpty())
            {
                out += QStringLiteral("\t\"NativeKeys\": [],") + eol;
            }
            else
            {
                out += QStringLiteral("\t\"NativeKeys\": [") + eol;
                for (int b = 0; b < a.bindings.size(); ++b)
                {
                    out += QStringLiteral("\t\t") + bindingText(a.bindings[b]);
                    if (b + 1 < a.bindings.size()) out += QLatin1Char(',');
                    out += eol;
                }
                out += QStringLiteral("\t],") + eol;
            }

            out += QStringLiteral("\t\"ValueType\": ") + q(valueTypeEngineName(a.valueType)) + QLatin1Char(',') + eol;
            for (const RawField& f : a.extra)
                out += QStringLiteral("\t") + q(f.name) + QStringLiteral(": ") + f.raw + QLatin1Char(',') + eol;
            out += QLatin1Char('}');
            if (i + 1 < map.actions.size()) out += eol;
        }
        return out;
    }

} // namespace imap
