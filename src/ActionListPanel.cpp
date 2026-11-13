#include "ActionListPanel.h"
#include "KesStyle.h"
#include "InputDocument.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace imap
{

    namespace
    {
        QString shortType(ValueType t)
        {
            switch (t)
            {
            case ValueType::Boolean: return QStringLiteral("Digital");
            case ValueType::Vector2: return QStringLiteral("Axis2D");
            case ValueType::Vector3: return QStringLiteral("Axis3D");
            }
            return {};
        }

        QString uniqueName(const InputMap& m, const QString& base)
        {
            auto taken = [&](const QString& n)
            { for (const Action& a : m.actions) if (a.name == n) return true; return false; };
            if (!taken(base)) return base;
            for (int i = 2;; ++i)
            {
                const QString n = QStringLiteral("%1%2").arg(base).arg(i);
                if (!taken(n)) return n;
            }
        }
    } // namespace

    ActionListPanel::ActionListPanel(InputDocument* doc, QWidget* parent) : QWidget(parent), doc_(doc)
    {
        auto* v = new QVBoxLayout(this);
        v->setContentsMargins(8, 8, 8, 8);
        v->setSpacing(6);

        auto* top = new QHBoxLayout;
        top->setSpacing(4);
        search_ = new QLineEdit(this);
        search_->setPlaceholderText(tr("Search actions"));
        search_->setClearButtonEnabled(true);
        search_->addAction(kes::style::icon(QStringLiteral("search")), QLineEdit::LeadingPosition);
        top->addWidget(search_, 1);

        auto mk = [this](const QString& icon, const QString& tip)
        {
            auto* b = new QToolButton(this);
            b->setIcon(kes::style::icon(icon));
            b->setToolTip(tip);
            b->setCursor(Qt::PointingHandCursor);
            return b;
        };
        addBtn_ = mk(QStringLiteral("plus"), tr("New input action"));
        dupBtn_ = mk(QStringLiteral("copy"), tr("Duplicate selected action"));
        delBtn_ = mk(QStringLiteral("trash"), tr("Delete selected action (Del)"));
        top->addWidget(addBtn_);
        top->addWidget(dupBtn_);
        top->addWidget(delBtn_);
        v->addLayout(top);

        tree_ = new QTreeWidget(this);
        tree_->setColumnCount(3);
        tree_->setHeaderLabels({ tr("Action"), tr("Type"), tr("Keys") });
        tree_->setRootIsDecorated(false);
        tree_->setAlternatingRowColors(true);
        tree_->setUniformRowHeights(true);
        tree_->setSelectionMode(QAbstractItemView::SingleSelection);
        tree_->setContextMenuPolicy(Qt::CustomContextMenu);
        tree_->header()->setStretchLastSection(false);
        tree_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        tree_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        tree_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        v->addWidget(tree_, 1);

        connect(addBtn_, &QToolButton::clicked, this, &ActionListPanel::addAction);
        connect(dupBtn_, &QToolButton::clicked, this, &ActionListPanel::duplicateCurrent);
        connect(delBtn_, &QToolButton::clicked, this, &ActionListPanel::deleteCurrent);
        connect(search_, &QLineEdit::textChanged, this, [this]
            { rebuild(); });

        connect(tree_, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* it)
            {
        if (rebuilding_) return;
        const int idx = it ? it->data(0, Qt::UserRole).toInt() : -1;
        if (idx == current_) return;
        current_ = idx;
        updateButtons();
        emit currentChanged(current_); });

        connect(tree_, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos)
            {
        QMenu menu(this);
        menu.addAction(tr("New action"), this, &ActionListPanel::addAction);
        if (tree_->itemAt(pos))
        {
            menu.addAction(tr("Duplicate"), this, &ActionListPanel::duplicateCurrent);
            menu.addAction(tr("Move up"), this, [this] { moveCurrent(-1); });
            menu.addAction(tr("Move down"), this, [this] { moveCurrent(+1); });
            menu.addSeparator();
            menu.addAction(tr("Delete"), this, &ActionListPanel::deleteCurrent);
        }
        menu.exec(tree_->viewport()->mapToGlobal(pos)); });

        tree_->installEventFilter(this);
        connect(doc_, &InputDocument::structureChanged, this, &ActionListPanel::rebuild);
        connect(doc_, &InputDocument::contentChanged, this, &ActionListPanel::refreshTexts);
        rebuild();
    }

    void ActionListPanel::setIssues(const QVector<Issue>& issues)
    {
        issues_ = issues;
        refreshTexts();
    }

    void ActionListPanel::updateButtons()
    {
        const bool has = current_ >= 0 && current_ < doc_->map().actions.size();
        dupBtn_->setEnabled(has);
        delBtn_->setEnabled(has);
    }

    void ActionListPanel::setCurrentIndex(int index)
    {
        for (int i = 0; i < tree_->topLevelItemCount(); ++i)
            if (tree_->topLevelItem(i)->data(0, Qt::UserRole).toInt() == index)
            {
                tree_->setCurrentItem(tree_->topLevelItem(i));
                return;
            }
    }

    void ActionListPanel::rebuild()
    {
        const InputMap& map = doc_->map();
        const int previous = current_;
        const QString needle = search_->text().trimmed().toLower();

        rebuilding_ = true;
        tree_->clear();
        QTreeWidgetItem* selectedItem = nullptr;
        QTreeWidgetItem* firstItem = nullptr;
        const int wanted = qMin(previous, map.actions.size() - 1);

        for (int i = 0; i < map.actions.size(); ++i)
        {
            const Action& a = map.actions[i];
            if (!needle.isEmpty() && !a.name.toLower().contains(needle)) continue;
            auto* item = new QTreeWidgetItem(tree_);
            item->setData(0, Qt::UserRole, i);
            item->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
            item->setForeground(1, kes::style::muted());
            item->setForeground(2, kes::style::muted());
            if (!firstItem) firstItem = item;
            if (i == wanted) selectedItem = item;
        }
        refreshTexts();

        if (!selectedItem) selectedItem = firstItem;
        const int target = selectedItem ? selectedItem->data(0, Qt::UserRole).toInt() : -1;
        if (selectedItem) tree_->setCurrentItem(selectedItem);
        rebuilding_ = false;

        current_ = target;
        updateButtons();
        if (target != previous) emit currentChanged(target);
    }

    bool ActionListPanel::eventFilter(QObject* watched, QEvent* e)
    {
        if (watched == tree_ && e->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(e)->key() == Qt::Key_Delete)
        {
            deleteCurrent();
            return true;
        }
        return QWidget::eventFilter(watched, e);
    }

    void ActionListPanel::refreshTexts()
    {
        const InputMap& map = doc_->map();
        for (int row = 0; row < tree_->topLevelItemCount(); ++row)
        {
            QTreeWidgetItem* item = tree_->topLevelItem(row);
            const int idx = item->data(0, Qt::UserRole).toInt();
            if (idx < 0 || idx >= map.actions.size()) continue;
            const Action& a = map.actions[idx];

            item->setText(0, a.name.isEmpty() ? tr("(unnamed)") : a.name);
            item->setText(1, shortType(a.valueType));
            item->setText(2, QString::number(a.bindings.size()));

            Severity worst = Severity::Info;
            bool any = false;
            for (const Issue& i : issues_)
                if (i.action == idx && i.severity != Severity::Info)
                {
                    any = true;
                    if (i.severity == Severity::Error)
                        worst = Severity::Error;
                    else if (worst != Severity::Error)
                        worst = Severity::Warning;
                }
            item->setIcon(0, any ? kes::style::icon(worst == Severity::Error ? QStringLiteral("error") : QStringLiteral("warning")) : QIcon());
        }
    }

    void ActionListPanel::addAction()
    {
        int newIndex = 0;
        doc_->edit(tr("Add input action"), true, [&](InputMap& m)
            {
        Action a;
        a.name = uniqueName(m, QStringLiteral("InputAction_NewAction"));
        a.valueType = ValueType::Boolean;
        newIndex = m.actions.size();
        m.actions.push_back(a); });
        search_->clear();
        setCurrentIndex(newIndex);
    }

    void ActionListPanel::duplicateCurrent()
    {
        const int i = current_;
        if (i < 0 || i >= doc_->map().actions.size()) return;
        int newIndex = i + 1;
        doc_->edit(tr("Duplicate input action"), true, [&](InputMap& m)
            {
        Action copy = m.actions[i];
        copy.name = uniqueName(m, copy.name + QStringLiteral("_Copy"));
        m.actions.insert(i + 1, copy); });
        setCurrentIndex(newIndex);
    }

    void ActionListPanel::deleteCurrent()
    {
        const int i = current_;
        if (i < 0 || i >= doc_->map().actions.size()) return;
        doc_->edit(tr("Delete input action"), true, [i](InputMap& m)
            { m.actions.removeAt(i); });
    }

    void ActionListPanel::moveCurrent(int delta)
    {
        const int i = current_, j = i + delta;
        if (i < 0 || j < 0 || i >= doc_->map().actions.size() || j >= doc_->map().actions.size()) return;
        doc_->edit(tr("Reorder input actions"), true, [i, j](InputMap& m)
            { m.actions.swapItemsAt(i, j); });
        search_->clear();
        setCurrentIndex(j);
    }

} // namespace imap
