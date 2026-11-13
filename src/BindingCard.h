#pragma once
#include "Validator.h"

#include <QFrame>

class QLabel;
class QToolButton;

namespace imap
{

    class InputDocument;
    class KeyPicker;
    class ElidedLabel;

    class BindingCard : public QFrame
    {
        Q_OBJECT
    public:
        BindingCard(InputDocument* doc, int actionIndex, int bindingIndex, bool expanded, QWidget* parent = nullptr);

        int bindingIndex() const { return bindingIndex_; }
        void setExpanded(bool expanded);

        void refresh(const QVector<Issue>& issuesForThisBinding);

    signals:
        void expandedChanged(int bindingIndex, bool expanded);
        void removeRequested(int bindingIndex);
        void duplicateRequested(int bindingIndex);
        void moveRequested(int bindingIndex, int delta);

    private:
        const Binding* binding() const;
        void updateSummaryVisibility();
        void buildBody(QWidget* body);
        QWidget* makeModifierRow(int modifierIndex, const Modifier& m);
        void editBinding(const QString& label, bool structural, const QString& mergeKey,
            const std::function<void(Binding&)>& change);

        InputDocument* doc_;
        int actionIndex_;
        int bindingIndex_;
        bool expanded_;

        QToolButton* chevron_ = nullptr;
        KeyPicker* picker_ = nullptr;
        ElidedLabel* summary_ = nullptr;
        QLabel* badge_ = nullptr;
        QWidget* body_ = nullptr;
    };

} // namespace imap
