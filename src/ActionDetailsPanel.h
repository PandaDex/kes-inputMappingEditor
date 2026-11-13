#pragma once
#include "Validator.h"

#include <QSet>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QScrollArea;

namespace imap
{

    class BindingCard;
    class InputDocument;

    class ActionDetailsPanel : public QWidget
    {
        Q_OBJECT
    public:
        explicit ActionDetailsPanel(InputDocument* doc, QWidget* parent = nullptr);

        void setActionIndex(int index);
        void setIssues(const QVector<Issue>& issues);
        void revealBinding(int bindingIndex);

    private:
        void rebuildLater();
        void rebuild();
        void refreshCards();
        void addMapping();
        void removeBinding(int bi);
        void duplicateBinding(int bi);
        void moveBinding(int bi, int delta);

        InputDocument* doc_;
        int index_ = -1;
        QVector<Issue> issues_;

        QScrollArea* scroll_;
        QLineEdit* nameEdit_ = nullptr;
        QComboBox* typeCombo_ = nullptr;
        QVector<BindingCard*> cards_;
        QSet<int> expanded_;
        bool rebuildQueued_ = false;
    };

} // namespace imap
