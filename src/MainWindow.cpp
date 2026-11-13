#include "MainWindow.h"
#include "ActionDetailsPanel.h"
#include "ActionListPanel.h"
#include "ImapFormat.h"
#include "KesStyle.h"
#include "InputDocument.h"
#include "PreviewPanel.h"
#include "ProblemsPanel.h"
#include "Validator.h"

#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>
#include <QDragEnterEvent>
#include <QDropEvent>

namespace imap
{
    namespace
    {
        const char* kFilter = "Input action maps (*.imap);;All files (*)";
        constexpr int kMaxRecent = 8;
    }

    MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
    {
        setAcceptDrops(true);
        resize(1380, 820);

        doc_ = new InputDocument(this);

        list_ = new ActionListPanel(doc_, this);
        details_ = new ActionDetailsPanel(doc_, this);
        preview_ = new PreviewPanel(this);
        problems_ = new ProblemsPanel(this);

        setCentralWidget(details_);

        auto* listDock = new QDockWidget(tr("Input Actions"), this);
        listDock->setObjectName(QStringLiteral("ActionsDock"));
        listDock->setWidget(list_);
        addDockWidget(Qt::LeftDockWidgetArea, listDock);

        auto* previewDock = new QDockWidget(tr("Generated .imap"), this);
        previewDock->setObjectName(QStringLiteral("PreviewDock"));
        previewDock->setWidget(preview_);
        addDockWidget(Qt::RightDockWidgetArea, previewDock);

        problemsDock_ = new QDockWidget(tr("Problems"), this);
        problemsDock_->setObjectName(QStringLiteral("ProblemsDock"));
        problemsDock_->setWidget(problems_);
        addDockWidget(Qt::BottomDockWidgetArea, problemsDock_);

        resizeDocks({ listDock, previewDock }, { 290, 400 }, Qt::Horizontal);
        resizeDocks({ problemsDock_ }, { 120 }, Qt::Vertical);

        statusLabel_ = new QLabel(this);
        issueLabel_ = new QLabel(this);
        statusBar()->addWidget(statusLabel_, 1);
        statusBar()->addPermanentWidget(issueLabel_);

        buildActions();
        buildMenusAndToolbar();

        connect(list_, &ActionListPanel::currentChanged, this, [this](int idx)
            {
            details_->setActionIndex(idx);
            const InputMap& m = doc_->map();
            preview_->highlightAction(idx >= 0 && idx < m.actions.size() ? m.actions[idx].name : QString(), true); });
        connect(problems_, &ProblemsPanel::locate, this, [this](int action, int binding)
            {
            list_->setCurrentIndex(action);
            if (binding >= 0) details_->revealBinding(binding); });

        connect(doc_, &InputDocument::structureChanged, this, &MainWindow::onDocumentChanged);
        connect(doc_, &InputDocument::contentChanged, this, &MainWindow::onDocumentChanged);
        connect(doc_, &InputDocument::modifiedChanged, this, &MainWindow::updateTitle);
        connect(doc_, &InputDocument::pathChanged, this, &MainWindow::updateTitle);

        QSettings s;
        restoreGeometry(s.value(QStringLiteral("window/geometry")).toByteArray());
        restoreState(s.value(QStringLiteral("window/state")).toByteArray());

        updateRecentMenu();
        onDocumentChanged();
        updateTitle();
    }

    MainWindow::~MainWindow()
    {
        if (doc_)
        {
            doc_->disconnect(this);
        }
    }

    void MainWindow::buildActions()
    {
        actNew_ = new QAction(kes::style::icon(QStringLiteral("new")), tr("&New"), this);
        actNew_->setShortcut(QKeySequence::New);
        actOpen_ = new QAction(kes::style::icon(QStringLiteral("open")), tr("&Open..."), this);
        actOpen_->setShortcut(QKeySequence::Open);
        actSave_ = new QAction(kes::style::icon(QStringLiteral("save")), tr("&Save"), this);
        actSave_->setShortcut(QKeySequence::Save);
        actSaveAs_ = new QAction(tr("Save &As..."), this);
        actSaveAs_->setShortcut(QKeySequence::SaveAs);
        actQuit_ = new QAction(tr("&Quit"), this);
        actQuit_->setShortcut(QKeySequence::Quit);

        actUndo_ = doc_->undoStack()->createUndoAction(this, tr("&Undo"));
        actUndo_->setShortcut(QKeySequence::Undo);
        actUndo_->setIcon(kes::style::icon(QStringLiteral("undo")));
        actRedo_ = doc_->undoStack()->createRedoAction(this, tr("&Redo"));
        actRedo_->setShortcut(QKeySequence::Redo);
        actRedo_->setIcon(kes::style::icon(QStringLiteral("redo")));

        actAdd_ = new QAction(kes::style::icon(QStringLiteral("plus")), tr("New Input &Action"), this);
        actAdd_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
        actDup_ = new QAction(kes::style::icon(QStringLiteral("copy")), tr("&Duplicate Action"), this);
        actDup_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
        actDel_ = new QAction(kes::style::icon(QStringLiteral("trash")), tr("D&elete Action"), this);

        connect(actNew_, &QAction::triggered, this, &MainWindow::newFile);
        connect(actOpen_, &QAction::triggered, this, &MainWindow::open);
        connect(actSave_, &QAction::triggered, this, [this]
            { save(); });
        connect(actSaveAs_, &QAction::triggered, this, [this]
            { saveAs(); });
        connect(actQuit_, &QAction::triggered, this, &QWidget::close);
        connect(actAdd_, &QAction::triggered, list_, &ActionListPanel::addAction);
        connect(actDup_, &QAction::triggered, list_, &ActionListPanel::duplicateCurrent);
        connect(actDel_, &QAction::triggered, list_, &ActionListPanel::deleteCurrent);
    }

    void MainWindow::buildMenusAndToolbar()
    {
        QMenu* file = menuBar()->addMenu(tr("&File"));
        file->addAction(actNew_);
        file->addAction(actOpen_);
        recentMenu_ = file->addMenu(tr("Open &Recent"));
        file->addSeparator();
        file->addAction(actSave_);
        file->addAction(actSaveAs_);
        file->addSeparator();
        file->addAction(actQuit_);

        QMenu* edit = menuBar()->addMenu(tr("&Edit"));
        edit->addAction(actUndo_);
        edit->addAction(actRedo_);
        edit->addSeparator();
        edit->addAction(actAdd_);
        edit->addAction(actDup_);
        edit->addAction(actDel_);

        QMenu* view = menuBar()->addMenu(tr("&View"));
        for (QDockWidget* d : findChildren<QDockWidget*>())
            view->addAction(d->toggleViewAction());

        QMenu* help = menuBar()->addMenu(tr("&Help"));
        help->addAction(tr("About"), this, [this]
            { QMessageBox::about(this, "About Input Mapping Editor", "GUI editor for KES Engine input mappings <br/>"
                                                                     "This application is part of <b>KES Engine</b> project."); });

        QToolBar* tb = addToolBar(tr("Main"));
        tb->setObjectName(QStringLiteral("MainToolbar"));
        tb->setMovable(false);
        tb->setIconSize(QSize(18, 18));
        tb->addAction(actNew_);
        tb->addAction(actOpen_);
        tb->addAction(actSave_);
        tb->addSeparator();
        tb->addAction(actUndo_);
        tb->addAction(actRedo_);
        tb->addSeparator();
        tb->addAction(actAdd_);
    }

    void MainWindow::onDocumentChanged()
    {
        const InputMap& map = doc_->map();
        const QVector<Issue> issues = validate(map, doc_->loadWarnings());

        QStringList names;
        int mappings = 0;
        for (const Action& a : map.actions)
        {
            names << a.name;
            mappings += a.bindings.size();
        }

        list_->setIssues(issues);
        details_->setIssues(issues);
        problems_->setIssues(issues, names);
        preview_->setText(doc_->generatedText());
        preview_->setFileLabel(doc_->filePath().isEmpty()
                ? tr("Not saved yet")
                : QFileInfo(doc_->filePath()).absoluteFilePath());

        int errors = 0, warnings = 0;
        for (const Issue& i : issues)
        {
            if (i.severity == Severity::Error)
                ++errors;
            else if (i.severity == Severity::Warning)
                ++warnings;
        }
        statusLabel_->setText(tr("%n input action(s), %1 mapping(s)", nullptr, map.actions.size()).arg(mappings));
        issueLabel_->setText(errors || warnings
                ? tr("%n error(s), %1 warning(s)", nullptr, errors).arg(warnings)
                : tr("No problems"));
        problemsDock_->setWindowTitle(errors || warnings ? tr("Problems (%1)").arg(errors + warnings) : tr("Problems"));

        actSave_->setEnabled(true);
        updateTitle();
    }

    void MainWindow::updateTitle()
    {
        setWindowTitle(kes::style::windowTitle(doc_->displayName(), tr("Input Mapping Editor")));
        setWindowModified(doc_->isModified());
    }

    bool MainWindow::maybeSave()
    {
        if (!doc_->isModified()) return true;
        const auto r = QMessageBox::warning(this, tr("Unsaved changes"),
            tr("Save changes to %1?").arg(doc_->displayName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        if (r == QMessageBox::Save) return save();
        return r == QMessageBox::Discard;
    }

    void MainWindow::newFile()
    {
        if (maybeSave()) doc_->newDocument();
    }

    void MainWindow::open()
    {
        if (!maybeSave()) return;
        const QString start = QSettings().value(QStringLiteral("lastDir")).toString();
        const QString path = QFileDialog::getOpenFileName(this, tr("Open input map"), start, tr(kFilter));
        if (!path.isEmpty()) openFile(path);
    }

    bool MainWindow::openFile(const QString& path)
    {
        QString err;
        if (!doc_->load(path, &err))
        {
            QMessageBox::critical(this, tr("Could not open file"), tr("%1\n\n%2").arg(path, err));
            return false;
        }
        QSettings().setValue(QStringLiteral("lastDir"), QFileInfo(path).absolutePath());
        addRecent(path);
        return true;
    }

    bool MainWindow::save()
    {
        return doc_->filePath().isEmpty() ? saveAs() : writeTo(doc_->filePath());
    }

    bool MainWindow::saveAs()
    {
        const QString start = doc_->filePath().isEmpty()
            ? QSettings().value(QStringLiteral("lastDir")).toString() + QStringLiteral("/default.imap")
            : doc_->filePath();
        const QString path = QFileDialog::getSaveFileName(this, tr("Save input map"), start, tr(kFilter));
        return !path.isEmpty() && writeTo(path);
    }

    bool MainWindow::writeTo(const QString& path)
    {
        int errors = 0;
        for (const Issue& i : validate(doc_->map(), doc_->loadWarnings()))
            if (i.severity == Severity::Error) ++errors;
        if (errors > 0)
        {
            const auto r = QMessageBox::warning(this, tr("Problems found"),
                tr(
                    "This map has %n error(s). Parts of it may not work in the engine.\n\nSave anyway?",
                    nullptr, errors),
                QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Cancel);
            if (r != QMessageBox::Save) return false;
        }

        QString err;
        if (!doc_->save(path, &err))
        {
            QMessageBox::critical(this, tr("Could not save file"), tr("%1\n\n%2").arg(path, err));
            return false;
        }
        QSettings().setValue(QStringLiteral("lastDir"), QFileInfo(path).absolutePath());
        addRecent(path);
        statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 3000);
        return true;
    }

    void MainWindow::addRecent(const QString& path)
    {
        QSettings s;
        QStringList recent = s.value(QStringLiteral("recent")).toStringList();
        recent.removeAll(path);
        recent.prepend(path);
        while (recent.size() > kMaxRecent)
            recent.removeLast();
        s.setValue(QStringLiteral("recent"), recent);
        updateRecentMenu();
    }

    void MainWindow::updateRecentMenu()
    {
        recentMenu_->clear();
        const QStringList recent = QSettings().value(QStringLiteral("recent")).toStringList();
        for (const QString& p : recent)
            recentMenu_->addAction(p, this, [this, p]
                { if (maybeSave()) openFile(p); });
        recentMenu_->setEnabled(!recent.isEmpty());
    }

    void MainWindow::closeEvent(QCloseEvent* e)
    {
        if (!maybeSave())
        {
            e->ignore();
            return;
        }
        QSettings s;
        s.setValue(QStringLiteral("window/geometry"), saveGeometry());
        s.setValue(QStringLiteral("window/state"), saveState());
        e->accept();
    }

    void MainWindow::dragEnterEvent(QDragEnterEvent* e)
    {
        if (e->mimeData()->hasUrls()) e->acceptProposedAction();
    }

    void MainWindow::dropEvent(QDropEvent* e)
    {
        const auto urls = e->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile() && maybeSave())
            openFile(urls.first().toLocalFile());
    }
} // namespace imap
