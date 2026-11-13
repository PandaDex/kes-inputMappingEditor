#pragma once
#include <QMainWindow>

class QDockWidget;
class QLabel;

namespace imap
{

    class ActionDetailsPanel;
    class ActionListPanel;
    class InputDocument;
    class PreviewPanel;
    class ProblemsPanel;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;

        bool openFile(const QString& path);
        InputDocument* document() const { return doc_; }

    protected:
        void closeEvent(QCloseEvent* e) override;
        void dragEnterEvent(QDragEnterEvent* e) override;
        void dropEvent(QDropEvent* e) override;

    private:
        void buildActions();
        void buildMenusAndToolbar();
        void onDocumentChanged();
        void updateTitle();
        void updateRecentMenu();
        void addRecent(const QString& path);

        bool maybeSave();
        bool save();
        bool saveAs();
        bool writeTo(const QString& path);
        void newFile();
        void open();

        InputDocument* doc_;
        ActionListPanel* list_;
        ActionDetailsPanel* details_;
        PreviewPanel* preview_;
        ProblemsPanel* problems_;
        QDockWidget* problemsDock_;
        QLabel* statusLabel_;
        QLabel* issueLabel_;

        QAction* actNew_;
        QAction* actOpen_;
        QAction* actSave_;
        QAction* actSaveAs_;
        QAction* actQuit_;
        QAction* actUndo_;
        QAction* actRedo_;
        QAction* actAdd_;
        QAction* actDup_;
        QAction* actDel_;
        QMenu* recentMenu_ = nullptr;
    };

} // namespace imap
