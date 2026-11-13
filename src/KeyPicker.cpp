#include "KeyPicker.h"
#include "KesStyle.h"
#include "KeyCatalog.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace imap
{

    namespace
    {
        QString categoryIconName(KeyCategory c)
        {
            switch (c)
            {
            case KeyCategory::Keyboard: return QStringLiteral("keyboard");
            case KeyCategory::Mouse:    return QStringLiteral("mouse");
            case KeyCategory::Gamepad:  return QStringLiteral("gamepad");
            }
            return {};
        }

        class DropButton : public QPushButton
        {
        public:
            using QPushButton::QPushButton;

        protected:
            void paintEvent(QPaintEvent* e) override
            {
                QPushButton::paintEvent(e);
                QPainter p(this);
                p.setRenderHint(QPainter::Antialiasing);
                p.setPen(QPen(isEnabled() ? kes::style::muted() : kes::style::disabled(), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                const qreal cx = width() - 13, cy = height() / 2.0;
                p.drawPolyline(QPolygonF({ { cx - 3.5, cy - 1.5 }, { cx, cy + 2 }, { cx + 3.5, cy - 1.5 } }));
            }
        };
    } // namespace

    CaptureButton::CaptureButton(QWidget* parent) : QToolButton(parent)
    {
        setIcon(kes::style::icon(QStringLiteral("capture")));
        setCheckable(false);
        setToolTip(tr("Listen for a key: click, then press the key you want to bind.\n"
                      "Click again (or anywhere else) to cancel. Only keys the engine supports can be captured."));
        setAutoRaise(false);
        setFocusPolicy(Qt::StrongFocus);
        connect(this, &QToolButton::clicked, this, [this]
            { listening_ ? stop() : start(); });
    }

    void CaptureButton::start()
    {
        if (listening_) return;
        listening_ = true;
        setProperty("listening", true);
        style()->unpolish(this);
        style()->polish(this);
        grabKeyboard();
        qApp->installEventFilter(this);
        emit listeningChanged(true);
    }

    void CaptureButton::stop()
    {
        if (!listening_) return;
        listening_ = false;
        setProperty("listening", false);
        style()->unpolish(this);
        style()->polish(this);
        releaseKeyboard();
        qApp->removeEventFilter(this);
        emit listeningChanged(false);
    }

    bool CaptureButton::event(QEvent* e)
    {
        if (listening_ && e->type() == QEvent::KeyPress)
        {
            auto* ke = static_cast<QKeyEvent*>(e);
            if (!ke->isAutoRepeat())
            {
                const QString name = KeyCatalog::nameFromKeyEvent(ke);
                if (!name.isEmpty())
                {
                    stop();
                    emit keyCaptured(name);
                }
                else
                {
                    emit keyRejected();
                }
            }
            return true; // swallow Tab etc. too
        }
        return QToolButton::event(e);
    }

    bool CaptureButton::eventFilter(QObject* watched, QEvent* e)
    {
        if (listening_ && e->type() == QEvent::MouseButtonPress)
        {
            const bool onSelf = (watched == this);
            stop();
            if (onSelf) return true; // otherwise the same click starts listening again
        }
        return QToolButton::eventFilter(watched, e);
    }

    void CaptureButton::hideEvent(QHideEvent* e)
    {
        stop();
        QToolButton::hideEvent(e);
    }

    KeyPopup::KeyPopup(QWidget* parent) : QFrame(parent, Qt::Popup)
    {
        setFrameShape(QFrame::StyledPanel);
        setObjectName(QStringLiteral("Popup"));

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setSpacing(6);

        search_ = new QLineEdit(this);
        search_->setPlaceholderText(tr("Search keys"));
        search_->setClearButtonEnabled(true);
        search_->addAction(kes::style::icon(QStringLiteral("search")), QLineEdit::LeadingPosition);
        layout->addWidget(search_);

        tree_ = new QTreeWidget(this);
        tree_->setHeaderHidden(true);
        tree_->setColumnCount(2);
        tree_->setRootIsDecorated(true);
        tree_->setIndentation(14);
        tree_->setUniformRowHeights(true);
        tree_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tree_->header()->setStretchLastSection(false);
        tree_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        tree_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        layout->addWidget(tree_, 1);

        for (KeyCategory cat : { KeyCategory::Keyboard, KeyCategory::Mouse, KeyCategory::Gamepad })
        {
            auto* group = new QTreeWidgetItem(tree_, { KeyCatalog::categoryName(cat) });
            group->setIcon(0, kes::style::icon(categoryIconName(cat)));
            group->setFlags(Qt::ItemIsEnabled);
            QFont f = group->font(0);
            f.setBold(true);
            group->setFont(0, f);
            for (const KeyInfo& k : KeyCatalog::all())
            {
                if (k.category != cat) continue;
                auto* item = new QTreeWidgetItem(group, { k.display, k.name });
                item->setData(0, Qt::UserRole, k.name);
                item->setForeground(1, kes::style::muted());
            }
            group->setExpanded(true);
        }

        connect(search_, &QLineEdit::textChanged, this, &KeyPopup::applyFilter);
        connect(tree_, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* it)
            { choose(it); });
        connect(tree_, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* it)
            { choose(it); });
        search_->installEventFilter(this);
    }

    void KeyPopup::choose(QTreeWidgetItem* item)
    {
        if (!item || !item->parent()) return;
        const QString key = item->data(0, Qt::UserRole).toString();
        close();
        emit keyChosen(key);
    }

    bool KeyPopup::eventFilter(QObject* watched, QEvent* e)
    {
        if (watched == search_ && e->type() == QEvent::KeyPress)
        {
            auto* ke = static_cast<QKeyEvent*>(e);
            switch (ke->key())
            {
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_PageDown:
            case Qt::Key_PageUp:
                QCoreApplication::sendEvent(tree_, e);
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                choose(tree_->currentItem());
                return true;
            default: break;
            }
        }
        return QFrame::eventFilter(watched, e);
    }

    void KeyPopup::applyFilter(const QString& text)
    {
        const QString needle = text.trimmed().toLower();
        QTreeWidgetItem* firstVisible = nullptr;
        for (int g = 0; g < tree_->topLevelItemCount(); ++g)
        {
            QTreeWidgetItem* group = tree_->topLevelItem(g);
            int visible = 0;
            for (int i = 0; i < group->childCount(); ++i)
            {
                QTreeWidgetItem* it = group->child(i);
                const bool match = needle.isEmpty() || it->text(0).toLower().contains(needle) || it->text(1).toLower().contains(needle);
                it->setHidden(!match);
                if (match)
                {
                    ++visible;
                    if (!firstVisible) firstVisible = it;
                }
            }
            group->setHidden(visible == 0);
            group->setExpanded(true);
        }
        if (firstVisible) tree_->setCurrentItem(firstVisible);
    }

    void KeyPopup::open(const QPoint& globalPos, int minWidth, const QString& currentKey)
    {
        search_->clear();
        applyFilter(QString());

        for (int g = 0; g < tree_->topLevelItemCount(); ++g)
            for (int i = 0; i < tree_->topLevelItem(g)->childCount(); ++i)
            {
                QTreeWidgetItem* it = tree_->topLevelItem(g)->child(i);
                if (it->data(0, Qt::UserRole).toString() == currentKey)
                {
                    tree_->setCurrentItem(it);
                    tree_->scrollToItem(it);
                }
            }

        resize(qMax(minWidth, 360), 330);
        move(globalPos);
        show();
        search_->setFocus();
    }

    KeyPicker::KeyPicker(QWidget* parent) : QWidget(parent)
    {
        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);

        button_ = new DropButton(this);
        button_->setObjectName(QStringLiteral("KeyButton"));
        button_->setCursor(Qt::PointingHandCursor);
        button_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button_->setToolTip(tr("Choose a key"));
        layout->addWidget(button_, 1);

        capture_ = new CaptureButton(this);
        layout->addWidget(capture_);

        connect(button_, &QPushButton::clicked, this, [this]
            { popup()->open(button_->mapToGlobal(QPoint(0, button_->height() + 2)), button_->width(), key_); });

        connect(capture_, &CaptureButton::listeningChanged, this, [this](bool on)
            {
        transientText_ = on ? tr("Press a key...") : QString();
        updateButton(); });
        connect(capture_, &CaptureButton::keyCaptured, this, &KeyPicker::keyChosen);
        connect(capture_, &CaptureButton::keyRejected, this, [this]
            {
        transientText_ = tr("Engine can't bind that key");
        updateButton();
        QTimer::singleShot(1200, this, [this] {
            transientText_ = capture_->isListening() ? tr("Press a key...") : QString();
            updateButton();
        }); });

        setKey(QString());
    }
    KeyPopup* KeyPicker::popup()
    {
        if (!popup_)
        {
            popup_ = new KeyPopup(this);
            connect(popup_, &KeyPopup::keyChosen, this, &KeyPicker::keyChosen);
        }
        return popup_;
    }

    void KeyPicker::setKey(const QString& engineKey)
    {
        key_ = engineKey;
        updateButton();
    }

    void KeyPicker::updateButton()
    {
        const KeyInfo* info = KeyCatalog::find(key_);
        const bool invalid = !info && transientText_.isEmpty();

        if (!transientText_.isEmpty())
        {
            button_->setText(transientText_);
            button_->setIcon(kes::style::icon(QStringLiteral("capture")));
        }
        else if (info)
        {
            button_->setText(info->display);
            button_->setIcon(kes::style::icon(categoryIconName(info->category)));
        }
        else
        {
            button_->setText(key_.isEmpty() ? tr("None") : tr("%1 (unsupported)").arg(key_));
            button_->setIcon(kes::style::icon(QStringLiteral("error")));
        }

        button_->setToolTip(info ? tr("%1\nEngine name: %2").arg(info->display, info->name) : tr("Choose a key"));
        button_->setProperty("invalid", invalid);
        button_->style()->unpolish(button_);
        button_->style()->polish(button_);
    }

} // namespace imap
