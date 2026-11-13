#pragma once
#include "KesStyle.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

namespace imap
{

    class Section : public QFrame
    {
    public:
        explicit Section(const QString& title, QWidget* parent = nullptr) : QFrame(parent)
        {
            setObjectName(QStringLiteral("Section"));

            auto* outer = new QVBoxLayout(this);
            outer->setContentsMargins(0, 0, 0, 0);
            outer->setSpacing(0);

            auto* header = new QFrame(this);
            header->setObjectName(QStringLiteral("SectionHeader"));
            headerLayout_ = new QHBoxLayout(header);
            headerLayout_->setContentsMargins(6, 4, 6, 4);
            headerLayout_->setSpacing(6);

            toggle_ = new QToolButton(header);
            toggle_->setAutoRaise(true);
            toggle_->setIcon(kes::style::icon(QStringLiteral("down")));
            toggle_->setCursor(Qt::PointingHandCursor);
            title_ = new QLabel(title, header);
            title_->setObjectName(QStringLiteral("SectionTitle"));
            headerLayout_->addWidget(toggle_);
            headerLayout_->addWidget(title_);
            headerLayout_->addStretch(1);

            body_ = new QWidget(this);
            bodyLayout_ = new QVBoxLayout(body_);
            bodyLayout_->setContentsMargins(10, 8, 10, 10);
            bodyLayout_->setSpacing(8);

            outer->addWidget(header);
            outer->addWidget(body_);

            QObject::connect(toggle_, &QToolButton::clicked, this, [this]
                { setExpanded(!body_->isVisible()); });
        }

        QVBoxLayout* bodyLayout() const { return bodyLayout_; }
        QHBoxLayout* headerLayout() const { return headerLayout_; }
        void setTitle(const QString& t) { title_->setText(t); }

        void setExpanded(bool on)
        {
            body_->setVisible(on);
            toggle_->setIcon(kes::style::icon(on ? QStringLiteral("down") : QStringLiteral("right")));
        }

    private:
        QToolButton* toggle_;
        QLabel* title_;
        QHBoxLayout* headerLayout_;
        QWidget* body_;
        QVBoxLayout* bodyLayout_;
    };

} // namespace imap
