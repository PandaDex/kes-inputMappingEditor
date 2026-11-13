#include "ProblemsPanel.h"
#include "KesStyle.h"

#include <QHeaderView>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace imap
{

    ProblemsPanel::ProblemsPanel(QWidget* parent) : QWidget(parent)
    {
        auto* v = new QVBoxLayout(this);
        v->setContentsMargins(0, 0, 0, 0);

        tree_ = new QTreeWidget(this);
        tree_->setColumnCount(2);
        tree_->setHeaderLabels({ tr("Problem"), tr("Where") });
        tree_->setRootIsDecorated(false);
        tree_->setAlternatingRowColors(true);
        tree_->setUniformRowHeights(true);
        tree_->header()->setStretchLastSection(false);
        tree_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        tree_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        tree_->setToolTip(tr("Double-click a problem to jump to it"));
        v->addWidget(tree_);

        connect(tree_, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* it)
            {
        const int a = it->data(0, Qt::UserRole).toInt();
        const int b = it->data(1, Qt::UserRole).toInt();
        if (a >= 0) emit locate(a, b); });
    }

    void ProblemsPanel::setIssues(const QVector<Issue>& issues, const QStringList& actionNames)
    {
        tree_->clear();
        auto rank = [](Severity s)
        { return s == Severity::Error ? 0 : s == Severity::Warning ? 1
                                                                   : 2; };
        QVector<Issue> sorted = issues;
        std::stable_sort(sorted.begin(), sorted.end(), [&](const Issue& a, const Issue& b)
            { return rank(a.severity) < rank(b.severity); });

        for (const Issue& i : sorted)
        {
            const char* icon = i.severity == Severity::Error ? "error" : i.severity == Severity::Warning ? "warning"
                                                                                                         : "info";
            QString where = i.action >= 0 && i.action < actionNames.size() ? actionNames[i.action] : tr("File");
            if (i.binding >= 0) where += tr(" / mapping %1").arg(i.binding + 1);

            auto* item = new QTreeWidgetItem(tree_, { i.message, where });
            item->setIcon(0, kes::style::icon(QString::fromLatin1(icon)));
            item->setToolTip(0, i.message);
            item->setData(0, Qt::UserRole, i.action);
            item->setData(1, Qt::UserRole, i.binding);
        }
    }

} // namespace imap
