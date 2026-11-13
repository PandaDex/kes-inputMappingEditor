#pragma once
#include "Validator.h"
#include <QWidget>

class QTreeWidget;

namespace imap
{

    class ProblemsPanel : public QWidget
    {
        Q_OBJECT
    public:
        explicit ProblemsPanel(QWidget* parent = nullptr);
        void setIssues(const QVector<Issue>& issues, const QStringList& actionNames);

    signals:
        void locate(int actionIndex, int bindingIndex);

    private:
        QTreeWidget* tree_;
    };

} // namespace imap
