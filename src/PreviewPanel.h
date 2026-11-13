#pragma once
#include <QWidget>

class QLabel;
class QPlainTextEdit;

namespace imap
{

    class PreviewPanel : public QWidget
    {
        Q_OBJECT
    public:
        explicit PreviewPanel(QWidget* parent = nullptr);

        void setText(const QString& text);
        void setFileLabel(const QString& text);
        void highlightAction(const QString& actionName, bool scrollTo);

    private:
        QPlainTextEdit* edit_;
        QLabel* fileLabel_;
        QString currentAction_;
    };

} // namespace imap
