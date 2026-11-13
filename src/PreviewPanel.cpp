#include "PreviewPanel.h"
#include "KesStyle.h"

#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QToolButton>
#include <QVBoxLayout>

namespace imap
{

    namespace
    {
        class ImapHighlighter : public QSyntaxHighlighter
        {
        public:
            using QSyntaxHighlighter::QSyntaxHighlighter;

        protected:
            void highlightBlock(const QString& text) override
            {
                static const QRegularExpression str(QStringLiteral("\"[^\"]*\""));
                static const QRegularExpression key(QStringLiteral("\"[^\"]*\"(?=\\s*:)"));
                static const QRegularExpression punct(QStringLiteral("[{}\\[\\]:,]"));

                QTextCharFormat value;
                value.setForeground(QColor(0x98, 0xc3, 0x79));
                QTextCharFormat name;
                name.setForeground(QColor(0x61, 0xaf, 0xef));
                QTextCharFormat sym;
                sym.setForeground(QColor(0x7f, 0x84, 0x8e));

                for (auto it = punct.globalMatch(text); it.hasNext();)
                {
                    auto m = it.next();
                    setFormat(m.capturedStart(), m.capturedLength(), sym);
                }
                for (auto it = str.globalMatch(text); it.hasNext();)
                {
                    auto m = it.next();
                    setFormat(m.capturedStart(), m.capturedLength(), value);
                }
                for (auto it = key.globalMatch(text); it.hasNext();)
                {
                    auto m = it.next();
                    setFormat(m.capturedStart(), m.capturedLength(), name);
                }
            }
        };
    } // namespace

    PreviewPanel::PreviewPanel(QWidget* parent) : QWidget(parent)
    {
        auto* v = new QVBoxLayout(this);
        v->setContentsMargins(8, 8, 8, 8);
        v->setSpacing(6);

        auto* top = new QHBoxLayout;
        fileLabel_ = new QLabel(this);
        fileLabel_->setObjectName(QStringLiteral("Muted"));
        fileLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        top->addWidget(fileLabel_, 1);

        auto* copy = new QToolButton(this);
        copy->setText(tr("Copy"));
        copy->setIcon(kes::style::icon(QStringLiteral("copy")));
        copy->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        copy->setToolTip(tr("Copy the generated file contents"));
        top->addWidget(copy);
        v->addLayout(top);

        edit_ = new QPlainTextEdit(this);
        edit_->setReadOnly(true);
        edit_->setLineWrapMode(QPlainTextEdit::NoWrap);
        edit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        edit_->setTabStopDistance(edit_->fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4);
        new ImapHighlighter(edit_->document());
        v->addWidget(edit_, 1);

        connect(copy, &QToolButton::clicked, this, [this]
            { QApplication::clipboard()->setText(edit_->toPlainText()); });
    }

    void PreviewPanel::setText(const QString& text)
    {
        if (edit_->toPlainText() == text) return;
        const int hs = edit_->horizontalScrollBar()->value();
        const int vs = edit_->verticalScrollBar()->value();
        edit_->setPlainText(text);
        edit_->horizontalScrollBar()->setValue(hs);
        edit_->verticalScrollBar()->setValue(vs);
        highlightAction(currentAction_, false);
    }

    void PreviewPanel::setFileLabel(const QString& text)
    {
        fileLabel_->setText(text);
        fileLabel_->setToolTip(text);
    }

    void PreviewPanel::highlightAction(const QString& actionName, bool scrollTo)
    {
        currentAction_ = actionName;
        QList<QTextEdit::ExtraSelection> selections;
        if (!actionName.isEmpty())
        {
            const QString header = QLatin1Char('"') + actionName + QStringLiteral("\": {");
            QTextBlock b = edit_->document()->firstBlock();
            for (; b.isValid(); b = b.next())
                if (b.text() == header) break;

            if (b.isValid())
            {
                QTextBlock last = b;
                while (last.isValid() && last.text() != QLatin1String("}"))
                    last = last.next();
                if (!last.isValid()) last = edit_->document()->lastBlock();

                QTextEdit::ExtraSelection sel;
                sel.cursor = QTextCursor(b);
                sel.cursor.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
                sel.format.setBackground(QColor(0x3a, 0x2c, 0x20));
                sel.format.setProperty(QTextFormat::FullWidthSelection, true);
                selections.push_back(sel);

                if (scrollTo)
                {
                    QTextCursor c(b);
                    edit_->setTextCursor(c);
                    edit_->centerCursor();
                }
            }
        }
        edit_->setExtraSelections(selections);
    }

} // namespace imap
