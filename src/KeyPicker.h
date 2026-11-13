#pragma once
#include <QFrame>
#include <QToolButton>
#include <QWidget>

class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace imap
{

    class CaptureButton : public QToolButton
    {
        Q_OBJECT
    public:
        explicit CaptureButton(QWidget* parent = nullptr);
        bool isListening() const { return listening_; }
        void start();
        void stop();

    signals:
        void keyCaptured(const QString& engineKey);
        void keyRejected();
        void listeningChanged(bool listening);

    protected:
        bool event(QEvent* e) override;
        bool eventFilter(QObject* watched, QEvent* e) override;
        void hideEvent(QHideEvent* e) override;

    private:
        bool listening_ = false;
    };

    class KeyPopup : public QFrame
    {
        Q_OBJECT
    public:
        explicit KeyPopup(QWidget* parent = nullptr);
        void open(const QPoint& globalPos, int minWidth, const QString& currentKey);

    signals:
        void keyChosen(const QString& engineKey);

    protected:
        bool eventFilter(QObject* watched, QEvent* e) override;

    private:
        void applyFilter(const QString& text);
        void choose(QTreeWidgetItem* item);

        QLineEdit* search_;
        QTreeWidget* tree_;
    };

    class KeyPicker : public QWidget
    {
        Q_OBJECT
    public:
        explicit KeyPicker(QWidget* parent = nullptr);
        void setKey(const QString& engineKey);
        QString key() const { return key_; }

    signals:
        void keyChosen(const QString& engineKey);

    private:
        void updateButton();
        KeyPopup* popup();

        QPushButton* button_;
        CaptureButton* capture_;
        KeyPopup* popup_ = nullptr;
        QString key_;
        QString transientText_;
    };

} // namespace imap
