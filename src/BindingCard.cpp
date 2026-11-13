#include "BindingCard.h"
#include "KesStyle.h"
#include "InputDocument.h"
#include "KeyPicker.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace imap
{

    class ElidedLabel : public QLabel
    {
    public:
        explicit ElidedLabel(QWidget* parent = nullptr) : QLabel(parent)
        {
            setObjectName(QStringLiteral("Muted"));
            setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
            setMinimumWidth(0);
        }
        QSize sizeHint() const override { return QSize(0, fontMetrics().height() + 2); }
        void setFullText(const QString& t)
        {
            full_ = t;
            setToolTip(t);
            update();
        }

    protected:
        void paintEvent(QPaintEvent*) override
        {
            QPainter p(this);
            p.setPen(kes::style::muted());
            const QRect r = contentsRect();
            p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, fontMetrics().elidedText(full_, Qt::ElideRight, r.width()));
        }

    private:
        QString full_;
    };

    namespace
    {

        QToolButton* iconButton(const QString& icon, const QString& tip, QWidget* parent)
        {
            auto* b = new QToolButton(parent);
            b->setIcon(kes::style::icon(icon));
            b->setToolTip(tip);
            b->setAutoRaise(true);
            b->setCursor(Qt::PointingHandCursor);
            return b;
        }

        QFrame* divider(QWidget* parent)
        {
            auto* f = new QFrame(parent);
            f->setObjectName(QStringLiteral("Divider"));
            f->setFrameShape(QFrame::NoFrame);
            return f;
        }

    } // namespace

    BindingCard::BindingCard(InputDocument* doc, int actionIndex, int bindingIndex, bool expanded, QWidget* parent)
        : QFrame(parent), doc_(doc), actionIndex_(actionIndex), bindingIndex_(bindingIndex), expanded_(expanded)
    {
        setObjectName(QStringLiteral("Card"));

        auto* outer = new QVBoxLayout(this);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->setSpacing(0);

        auto* header = new QFrame(this);
        header->setObjectName(QStringLiteral("CardHeader"));
        auto* headerV = new QVBoxLayout(header);
        headerV->setContentsMargins(6, 6, 6, 6);
        headerV->setSpacing(2);
        auto* h = new QHBoxLayout;
        h->setSpacing(4);
        headerV->addLayout(h);

        chevron_ = iconButton(QStringLiteral("right"), tr("Show or hide details"), header);
        h->addWidget(chevron_);

        picker_ = new KeyPicker(header);
        picker_->setMinimumWidth(190);
        h->addWidget(picker_, 1);

        badge_ = new QLabel(header);
        badge_->setFixedWidth(18);
        h->addWidget(badge_);

        auto* up = iconButton(QStringLiteral("up"), tr("Move up"), header);
        auto* down = iconButton(QStringLiteral("down"), tr("Move down"), header);
        auto* dup = iconButton(QStringLiteral("copy"), tr("Duplicate mapping"), header);
        auto* del = iconButton(QStringLiteral("trash"), tr("Remove mapping"), header);
        for (auto* b : { up, down, dup, del })
            h->addWidget(b);

        summary_ = new ElidedLabel(header);
        summary_->setContentsMargins(30, 0, 0, 0);
        headerV->addWidget(summary_);

        outer->addWidget(header);

        body_ = new QWidget(this);
        buildBody(body_);
        outer->addWidget(body_);

        connect(chevron_, &QToolButton::clicked, this, [this]
            { setExpanded(!expanded_); emit expandedChanged(bindingIndex_, expanded_); });
        connect(up, &QToolButton::clicked, this, [this]
            { emit moveRequested(bindingIndex_, -1); });
        connect(down, &QToolButton::clicked, this, [this]
            { emit moveRequested(bindingIndex_, +1); });
        connect(dup, &QToolButton::clicked, this, [this]
            { emit duplicateRequested(bindingIndex_); });
        connect(del, &QToolButton::clicked, this, [this]
            { emit removeRequested(bindingIndex_); });

        connect(picker_, &KeyPicker::keyChosen, this, [this](const QString& key)
            { editBinding(tr("Change key"), true, QString(), [key](Binding& b)
                  { b.key = key; }); });

        if (const Binding* b = binding()) picker_->setKey(b->key);
        setExpanded(expanded_);
        refresh({});
    }

    void BindingCard::updateSummaryVisibility()
    {
        const Binding* b = binding();
        summary_->setVisible(!expanded_ && b && !b->modifiers.isEmpty());
    }

    const Binding* BindingCard::binding() const
    {
        const InputMap& m = doc_->map();
        if (actionIndex_ < 0 || actionIndex_ >= m.actions.size()) return nullptr;
        const Action& a = m.actions[actionIndex_];
        if (bindingIndex_ < 0 || bindingIndex_ >= a.bindings.size()) return nullptr;
        return &a.bindings[bindingIndex_];
    }

    void BindingCard::editBinding(const QString& label, bool structural, const QString& mergeKey,
        const std::function<void(Binding&)>& change)
    {
        const int ai = actionIndex_, bi = bindingIndex_;
        doc_->edit(label, structural, [=](InputMap& m)
            {
        if (ai < m.actions.size() && bi < m.actions[ai].bindings.size())
            change(m.actions[ai].bindings[bi]); }, mergeKey);
    }

    void BindingCard::setExpanded(bool expanded)
    {
        expanded_ = expanded;
        body_->setVisible(expanded);
        updateSummaryVisibility();
        chevron_->setIcon(kes::style::icon(expanded ? QStringLiteral("down") : QStringLiteral("right")));
        setProperty("expanded", expanded);
        style()->unpolish(this);
        style()->polish(this);
    }

    void BindingCard::refresh(const QVector<Issue>& issues)
    {
        const Binding* b = binding();
        if (!b) return;

        QStringList parts;
        for (const Modifier& m : b->modifiers)
            parts << modifierSummary(m);
        summary_->setFullText(parts.join(QStringLiteral(", ")));
        updateSummaryVisibility();

        Severity worst = Severity::Info;
        QStringList lines;
        for (const Issue& i : issues)
        {
            if (static_cast<int>(i.severity) > static_cast<int>(worst)) worst = i.severity;
            lines << i.message;
        }
        if (issues.isEmpty())
        {
            badge_->clear();
            badge_->setToolTip(QString());
        }
        else
        {
            const char* icon = worst == Severity::Error ? "error" : worst == Severity::Warning ? "warning"
                                                                                               : "info";
            badge_->setPixmap(kes::style::icon(QString::fromLatin1(icon)).pixmap(16, 16));
            badge_->setToolTip(lines.join(QStringLiteral("\n\n")));
        }
    }

    void BindingCard::buildBody(QWidget* body)
    {
        const Binding* b = binding();
        auto* v = new QVBoxLayout(body);
        v->setContentsMargins(10, 0, 10, 10);
        v->setSpacing(8);
        v->addWidget(divider(body));
        if (!b) return;

        auto* modHeader = new QHBoxLayout;
        auto* modTitle = new QLabel(tr("Modifiers"), body);
        modTitle->setObjectName(QStringLiteral("SectionTitle"));
        modTitle->setToolTip(tr("Applied top to bottom, so the order changes the result."));
        modHeader->addWidget(modTitle);
        modHeader->addStretch(1);

        auto* addMod = new QToolButton(body);
        addMod->setText(tr("Add modifier"));
        addMod->setIcon(kes::style::icon(QStringLiteral("plus")));
        addMod->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        addMod->setPopupMode(QToolButton::InstantPopup);
        addMod->setCursor(Qt::PointingHandCursor);
        auto* menu = new QMenu(addMod);
        for (const ModifierInfo& info : modifierRegistry())
        {
            QAction* act = menu->addAction(QString::fromLatin1(info.displayName));
            act->setToolTip(QString::fromLatin1(info.tooltip));
            const ModifierType type = info.type;
            connect(act, &QAction::triggered, this, [this, type]
                { editBinding(tr("Add modifier"), true, QString(), [type](Binding& bd)
                      { bd.modifiers.push_back(makeDefaultModifier(type)); }); });
        }
        menu->setToolTipsVisible(true);
        addMod->setMenu(menu);
        modHeader->addWidget(addMod);
        v->addLayout(modHeader);

        if (b->modifiers.isEmpty())
        {
            auto* none = new QLabel(tr("No modifiers. The raw key value is used as-is."), body);
            none->setObjectName(QStringLiteral("Muted"));
            v->addWidget(none);
        }
        for (int mi = 0; mi < b->modifiers.size(); ++mi)
            v->addWidget(makeModifierRow(mi, b->modifiers[mi]));

        v->addWidget(divider(body));

        auto* pmTitle = new QLabel(tr("Player Mappable"), body);
        pmTitle->setObjectName(QStringLiteral("SectionTitle"));
        v->addWidget(pmTitle);

        auto* pm = new QHBoxLayout;
        pm->setSpacing(8);
        auto* assignable = new QCheckBox(tr("Player assignable"), body);
        assignable->setChecked(b->playerAssignable);
        assignable->setToolTip(tr("Written as bPlayerAssignable."));
        auto* keyLabel = new QLabel(tr("Assignable key"), body);
        keyLabel->setObjectName(QStringLiteral("Muted"));
        auto* keyEdit = new QLineEdit(b->assignableKey, body);
        keyEdit->setToolTip(tr("Written as AssignableKey. \"none\" means no player-chosen key yet."));
        pm->addWidget(assignable);
        pm->addStretch(1);
        pm->addWidget(keyLabel);
        pm->addWidget(keyEdit, 1);
        v->addLayout(pm);

        connect(assignable, &QCheckBox::toggled, this, [this](bool on)
            { editBinding(tr("Change player assignable"), false, QString(), [on](Binding& bd)
                  { bd.playerAssignable = on; }); });
        connect(keyEdit, &QLineEdit::editingFinished, this, [this, keyEdit]
            {
        const QString text = keyEdit->text();
        editBinding(tr("Change assignable key"), false, QString(), [text](Binding& bd) { bd.assignableKey = text; }); });
    }

    QWidget* BindingCard::makeModifierRow(int mi, const Modifier& m)
    {
        auto* row = new QFrame(body_);
        row->setObjectName(QStringLiteral("ModifierRow"));
        auto* v = new QVBoxLayout(row);
        v->setContentsMargins(8, 6, 6, 8);
        v->setSpacing(6);

        auto* top = new QHBoxLayout;
        top->setSpacing(2);
        const ModifierInfo* info = modifierInfo(m.type);
        auto* title = new QLabel(info ? QString::fromLatin1(info->displayName)
                                      : (m.unknownName.isEmpty() ? tr("Unknown modifier") : m.unknownName),
            row);
        title->setObjectName(QStringLiteral("SectionTitle"));
        if (info) title->setToolTip(QString::fromLatin1(info->tooltip));
        top->addWidget(title);
        top->addStretch(1);
        auto* up = iconButton(QStringLiteral("up"), tr("Apply earlier"), row);
        auto* down = iconButton(QStringLiteral("down"), tr("Apply later"), row);
        auto* del = iconButton(QStringLiteral("trash"), tr("Remove modifier"), row);
        for (auto* b : { up, down, del })
            top->addWidget(b);
        v->addLayout(top);

        connect(up, &QToolButton::clicked, this, [this, mi]
            { editBinding(tr("Reorder modifier"), true, QString(), [mi](Binding& b)
                  {
            if (mi > 0 && mi < b.modifiers.size()) b.modifiers.swapItemsAt(mi, mi - 1); }); });
        connect(down, &QToolButton::clicked, this, [this, mi]
            { editBinding(tr("Reorder modifier"), true, QString(), [mi](Binding& b)
                  {
            if (mi >= 0 && mi + 1 < b.modifiers.size()) b.modifiers.swapItemsAt(mi, mi + 1); }); });
        connect(del, &QToolButton::clicked, this, [this, mi]
            { editBinding(tr("Remove modifier"), true, QString(), [mi](Binding& b)
                  {
            if (mi >= 0 && mi < b.modifiers.size()) b.modifiers.removeAt(mi); }); });

        switch (m.type)
        {
        case ModifierType::Deadzone:
        {
            auto* r = new QHBoxLayout;
            auto* l = new QLabel(tr("Lower threshold"), row);
            l->setObjectName(QStringLiteral("Muted"));
            auto* spin = new QDoubleSpinBox(row);
            spin->setRange(0.0, 1.0);
            spin->setDecimals(3);
            spin->setSingleStep(0.01);
            spin->setValue(m.scalar);
            spin->setKeyboardTracking(false);
            r->addWidget(l);
            r->addStretch(1);
            r->addWidget(spin);
            v->addLayout(r);
            const QString key = QStringLiteral("dz:%1:%2:%3").arg(actionIndex_).arg(bindingIndex_).arg(mi);
            connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, mi, key](double val)
                { editBinding(tr("Change dead zone"), false, key, [mi, val](Binding& b)
                      {
                if (mi < b.modifiers.size()) b.modifiers[mi].scalar = val; }); });
            break;
        }
        case ModifierType::Negate:
        {
            auto* r = new QHBoxLayout;
            auto* l = new QLabel(tr("Negate axes"), row);
            l->setObjectName(QStringLiteral("Muted"));
            r->addWidget(l);
            r->addStretch(1);
            struct Axis
            {
                const char* name;
                bool value;
                bool Modifier::* field;
            };
            const Axis axes[] = { { "X", m.negateX, &Modifier::negateX },
                { "Y", m.negateY, &Modifier::negateY },
                { "Z", m.negateZ, &Modifier::negateZ } };
            for (const Axis& axis : axes)
            {
                auto* cb = new QCheckBox(QString::fromLatin1(axis.name), row);
                cb->setChecked(axis.value);
                auto field = axis.field;
                connect(cb, &QCheckBox::toggled, this, [this, mi, field](bool on)
                    { editBinding(tr("Change negate"), false, QString(), [mi, field, on](Binding& b)
                          {
                    if (mi < b.modifiers.size()) b.modifiers[mi].*field = on; }); });
                r->addWidget(cb);
            }
            v->addLayout(r);
            break;
        }
        case ModifierType::Unknown:
        {
            auto* l = new QLabel(tr("Not supported by the engine. Kept in the file unchanged."), row);
            l->setObjectName(QStringLiteral("Muted"));
            l->setWordWrap(true);
            v->addWidget(l);
            break;
        }
        }
        return row;
    }

} // namespace imap
