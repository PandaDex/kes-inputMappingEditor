#include "InputDocument.h"
#include "ImapFormat.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace imap
{

    class SnapshotCommand : public QUndoCommand
    {
    public:
        SnapshotCommand(InputDocument* doc, InputMap before, InputMap after, bool structural,
            const QString& label, const QString& mergeKey)
            : QUndoCommand(label), doc_(doc), before_(std::move(before)), after_(std::move(after)), structural_(structural), mergeKey_(mergeKey) {}

        void redo() override { doc_->applySnapshot(after_, structural_); }
        void undo() override { doc_->applySnapshot(before_, true); }

        int id() const override { return mergeKey_.isEmpty() ? -1 : 1; }

        bool mergeWith(const QUndoCommand* other) override
        {
            const auto* o = dynamic_cast<const SnapshotCommand*>(other);
            if (!o || o->doc_ != doc_ || o->mergeKey_ != mergeKey_ || o->structural_ != structural_)
                return false;
            after_ = o->after_;
            return true;
        }

    private:
        InputDocument* doc_;
        InputMap before_, after_;
        bool structural_;
        QString mergeKey_;
    };

    InputDocument::InputDocument(QObject* parent) : QObject(parent)
    {
        connect(&undo_, &QUndoStack::cleanChanged, this, &InputDocument::modifiedChanged);
    }

    QString InputDocument::displayName() const
    {
        return path_.isEmpty() ? QStringLiteral("Untitled.imap") : QFileInfo(path_).fileName();
    }

    void InputDocument::applySnapshot(const InputMap& map, bool structural)
    {
        map_ = map;
        if (structural)
            emit structureChanged();
        else
            emit contentChanged();
    }

    void InputDocument::edit(const QString& label, bool structural,
        const std::function<void(InputMap&)>& change, const QString& mergeKey)
    {
        InputMap after = map_;
        change(after);
        if (after == map_) return;
        undo_.push(new SnapshotCommand(this, map_, std::move(after), structural, label, mergeKey));
    }

    void InputDocument::newDocument()
    {
        map_ = InputMap{};
        path_.clear();
        loadWarnings_.clear();
        undo_.clear();
        emit pathChanged();
        emit structureChanged();
        emit modifiedChanged();
    }

    bool InputDocument::load(const QString& path, QString* error)
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
        {
            if (error) *error = f.errorString();
            return false;
        }

        const LoadResult r = parseImap(QString::fromUtf8(f.readAll()));
        if (!r.ok)
        {
            if (error) *error = r.error;
            return false;
        }

        map_ = r.map;
        loadWarnings_ = r.warnings;
        path_ = path;
        undo_.clear();
        emit pathChanged();
        emit structureChanged();
        emit modifiedChanged();
        return true;
    }

    QString InputDocument::generatedText() const
    {
        return writeImap(map_, QStringLiteral("\n"));
    }

    bool InputDocument::save(const QString& path, QString* error)
    {
        QSaveFile f(path);
        if (!f.open(QIODevice::WriteOnly))
        {
            if (error) *error = f.errorString();
            return false;
        }

        f.write(writeImap(map_, map_.crlf ? QStringLiteral("\r\n") : QStringLiteral("\n")).toUtf8());
        if (!f.commit())
        {
            if (error) *error = f.errorString();
            return false;
        }

        const bool pathChangedNow = (path != path_);
        path_ = path;
        undo_.setClean();
        if (pathChangedNow) emit pathChanged();
        emit modifiedChanged();
        return true;
    }

} // namespace imap
