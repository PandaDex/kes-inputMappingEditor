#include "ActionDetailsPanel.h"
#include "BindingCard.h"
#include "KesStyle.h"
#include "InputDocument.h"
#include "KeyCatalog.h"
#include "Section.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace imap
{

    ActionDetailsPanel::ActionDetailsPanel(InputDocument* doc, QWidget* parent) : QWidget(parent), doc_(doc)
    {
        auto* v = new QVBoxLayout(this);
        v->setContentsMargins(0, 0, 0, 0);
        scroll_ = new QScrollArea(this);
        scroll_->setWidgetResizable(true);
        scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        v->addWidget(scroll_);

        connect(doc_, &InputDocument::structureChanged, this, &ActionDetailsPanel::rebuildLater);
        connect(doc_, &InputDocument::contentChanged, this, [this]
            {
        const InputMap& m = doc_->map();
        refreshCards(); });
        rebuild();
    }

    void ActionDetailsPanel::setActionIndex(int index)
    {
        if (index == index_) return;
        index_ = index;
        expanded_.clear();
        if (index_ >= 0 && index_ < doc_->map().actions.size())
        {
            const int n = doc_->map().actions[index_].bindings.size();
            if (n <= 3)
                for (int i = 0; i < n; ++i)
                    expanded_.insert(i);
        }
        rebuildLater();
    }

    void ActionDetailsPanel::setIssues(const QVector<Issue>& issues)
    {
        issues_ = issues;
        refreshCards();
    }

    void ActionDetailsPanel::refreshCards()
    {
        for (BindingCard* c : cards_)
        {
            QVector<Issue> mine;
            for (const Issue& i : issues_)
                if (i.action == index_ && i.binding == c->bindingIndex()) mine.push_back(i);
            c->refresh(mine);
        }
    }

    void ActionDetailsPanel::rebuildLater()
    {
        if (rebuildQueued_) return;
        rebuildQueued_ = true;
        // signal can come from a widget inside the panel we're about to delete
        QTimer::singleShot(0, this, [this]
            { rebuildQueued_ = false; rebuild(); });
    }

    void ActionDetailsPanel::rebuild()
    {
        const int scrollPos = scroll_->verticalScrollBar()->value();
        cards_.clear();

        auto* content = new QWidget;
        auto* v = new QVBoxLayout(content);
        v->setContentsMargins(12, 12, 12, 12);
        v->setSpacing(10);

        const InputMap& map = doc_->map();
        if (index_ < 0 || index_ >= map.actions.size())
        {
            auto* empty = new QLabel(map.actions.isEmpty()
                    ? tr("No input actions yet.\nUse + in the Input Actions panel to create one.")
                    : tr("Select an input action to edit its mappings."),
                content);
            empty->setObjectName(QStringLiteral("EmptyState"));
            empty->setAlignment(Qt::AlignCenter);
            v->addStretch(1);
            v->addWidget(empty);
            v->addStretch(1);
            if (QWidget* old = scroll_->takeWidget()) old->deleteLater();
            scroll_->setWidget(content);
            return;
        }

        const Action& action = map.actions[index_];

        auto* actionSection = new Section(tr("Action"), content);
        auto* form = new QFormLayout;
        form->setLabelAlignment(Qt::AlignLeft);
        form->setHorizontalSpacing(14);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

        nameEdit_ = new QLineEdit(action.name, actionSection);
        nameEdit_->setToolTip(tr("Name used by the game code, e.g. InputActionDown(input, \"InputAction_Move\")."));
        form->addRow(tr("Name"), nameEdit_);

        typeCombo_ = new QComboBox(actionSection);
        for (ValueType t : { ValueType::Boolean, ValueType::Vector2, ValueType::Vector3 })
            typeCombo_->addItem(valueTypeDisplayName(t), static_cast<int>(t));
        typeCombo_->setCurrentIndex(static_cast<int>(action.valueType));
        typeCombo_->setToolTip(tr("Written as ValueType: boolean, vector2 or vector3."));
        form->addRow(tr("Value type"), typeCombo_);
        actionSection->bodyLayout()->addLayout(form);
        v->addWidget(actionSection);

        const int ai = index_;
        connect(nameEdit_, &QLineEdit::editingFinished, this, [this, ai]
            {
        if (!nameEdit_) return;
        const QString text = nameEdit_->text();
        doc_->edit(tr("Rename input action"), false, [ai, text](InputMap& m) {
            if (ai < m.actions.size()) m.actions[ai].name = text; }); });
        connect(typeCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, ai](int idx)
            {
        const ValueType t = static_cast<ValueType>(idx);
        doc_->edit(tr("Change value type"), false, [ai, t](InputMap& m) {
            if (ai < m.actions.size()) m.actions[ai].valueType = t; }); });

        auto* mappings = new Section(tr("Mappings (%1)").arg(action.bindings.size()), content);
        auto* add = new QToolButton(mappings);
        add->setText(tr("Add mapping"));
        add->setIcon(kes::style::icon(QStringLiteral("plus")));
        add->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        add->setCursor(Qt::PointingHandCursor);
        mappings->headerLayout()->addWidget(add);
        connect(add, &QToolButton::clicked, this, &ActionDetailsPanel::addMapping);

        if (action.bindings.isEmpty())
        {
            auto* hint = new QLabel(tr("No mappings. Add one to bind a key, mouse axis or gamepad input to this action."), mappings);
            hint->setObjectName(QStringLiteral("Muted"));
            hint->setWordWrap(true);
            mappings->bodyLayout()->addWidget(hint);
        }

        for (int bi = 0; bi < action.bindings.size(); ++bi)
        {
            auto* card = new BindingCard(doc_, ai, bi, expanded_.contains(bi), mappings);
            connect(card, &BindingCard::expandedChanged, this, [this](int i, bool on)
                {
            if (on) expanded_.insert(i); else expanded_.remove(i); });
            connect(card, &BindingCard::removeRequested, this, &ActionDetailsPanel::removeBinding);
            connect(card, &BindingCard::duplicateRequested, this, &ActionDetailsPanel::duplicateBinding);
            connect(card, &BindingCard::moveRequested, this, &ActionDetailsPanel::moveBinding);
            mappings->bodyLayout()->addWidget(card);
            cards_.push_back(card);
        }
        v->addWidget(mappings);
        v->addStretch(1);

        if (QWidget* old = scroll_->takeWidget()) old->deleteLater();
        scroll_->setWidget(content);
        refreshCards();
        QTimer::singleShot(0, this, [this, scrollPos]
            { scroll_->verticalScrollBar()->setValue(scrollPos); });
    }

    void ActionDetailsPanel::addMapping()
    {
        const int ai = index_;
        if (ai < 0 || ai >= doc_->map().actions.size()) return;
        const int newIndex = doc_->map().actions[ai].bindings.size();
        expanded_.insert(newIndex);
        doc_->edit(tr("Add mapping"), true, [ai](InputMap& m)
            {
        Binding b;
        b.key = (m.actions[ai].valueType == ValueType::Boolean)
                    ? QStringLiteral("Keyboard_Space") : QStringLiteral("Gamepad_LeftAxis_2D");
        m.actions[ai].bindings.push_back(b); });
        revealBinding(newIndex);
    }

    void ActionDetailsPanel::removeBinding(int bi)
    {
        const int ai = index_;
        QSet<int> shifted;
        for (int e : std::as_const(expanded_))
        {
            if (e < bi)
                shifted.insert(e);
            else if (e > bi)
                shifted.insert(e - 1);
        }
        expanded_ = shifted;
        doc_->edit(tr("Remove mapping"), true, [ai, bi](InputMap& m)
            {
        if (ai < m.actions.size() && bi < m.actions[ai].bindings.size()) m.actions[ai].bindings.removeAt(bi); });
    }

    void ActionDetailsPanel::duplicateBinding(int bi)
    {
        const int ai = index_;
        QSet<int> shifted;
        for (int e : std::as_const(expanded_))
            shifted.insert(e > bi ? e + 1 : e);
        shifted.insert(bi + 1);
        expanded_ = shifted;
        doc_->edit(tr("Duplicate mapping"), true, [ai, bi](InputMap& m)
            {
        if (ai < m.actions.size() && bi < m.actions[ai].bindings.size())
            m.actions[ai].bindings.insert(bi + 1, m.actions[ai].bindings[bi]); });
    }

    void ActionDetailsPanel::moveBinding(int bi, int delta)
    {
        const int ai = index_, bj = bi + delta;
        if (ai < 0 || ai >= doc_->map().actions.size()) return;
        if (bj < 0 || bj >= doc_->map().actions[ai].bindings.size()) return;
        const bool a = expanded_.contains(bi), b = expanded_.contains(bj);
        expanded_.remove(bi);
        expanded_.remove(bj);
        if (a) expanded_.insert(bj);
        if (b) expanded_.insert(bi);
        doc_->edit(tr("Reorder mapping"), true, [ai, bi, bj](InputMap& m)
            { m.actions[ai].bindings.swapItemsAt(bi, bj); });
    }

    void ActionDetailsPanel::revealBinding(int bi)
    {
        expanded_.insert(bi);
        rebuildLater();
        QTimer::singleShot(30, this, [this, bi]
            {
        for (BindingCard* c : cards_)
            if (c->bindingIndex() == bi) scroll_->ensureWidgetVisible(c, 0, 24); });
    }

} // namespace imap
