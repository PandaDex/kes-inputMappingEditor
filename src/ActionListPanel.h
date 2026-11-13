#pragma once
#include "Validator.h"
#include <QWidget>

class QLineEdit;
class QToolButton;
class QTreeWidget;

namespace imap
{

    class InputDocument;

    class ActionListPanel : public QWidget
    {
        Q_OBJECT
    public:
        explicit ActionListPanel(InputDocument* doc, QWidget* parent = nullptr);

        int currentIndex() const { return current_; }
        void setCurrentIndex(int index);
        void setIssues(const QVector<Issue>& issues);

        void addAction();
        void duplicateCurrent();
        void deleteCurrent();
        void moveCurrent(int delta);

    signals:
        void currentChanged(int index);

    protected:
        bool eventFilter(QObject* watched, QEvent* e) override;

    private:
        void rebuild();
        void refreshTexts();
        void updateButtons();

        InputDocument* doc_;
        QLineEdit* search_;
        QTreeWidget* tree_;
        QToolButton* addBtn_;
        QToolButton* dupBtn_;
        QToolButton* delBtn_;
        int current_ = -1;
        bool rebuilding_ = false;
        QVector<Issue> issues_;
    };

} // namespace imap
