#pragma once
#include "InputModel.h"
#include <QObject>
#include <QUndoStack>
#include <functional>

namespace imap
{

    class InputDocument : public QObject
    {
        Q_OBJECT
    public:
        explicit InputDocument(QObject* parent = nullptr);

        const InputMap& map() const { return map_; }
        QUndoStack* undoStack() { return &undo_; }

        QString filePath() const { return path_; }
        QString displayName() const;
        bool isModified() const { return !undo_.isClean(); }
        const QStringList& loadWarnings() const { return loadWarnings_; }

        void newDocument();
        bool load(const QString& path, QString* error);
        bool save(const QString& path, QString* error);

        QString generatedText() const;

        // structural: actions/mappings/modifiers added, removed or reordered -> panels rebuild.
        // Otherwise widgets are kept so focus survives typing.
        // Consecutive edits with the same mergeKey become one undo step.
        void edit(const QString& label, bool structural,
            const std::function<void(InputMap&)>& change,
            const QString& mergeKey = QString());

    signals:
        void structureChanged();
        void contentChanged();
        void modifiedChanged();
        void pathChanged();

    private:
        friend class SnapshotCommand;
        void applySnapshot(const InputMap& map, bool structural);

        InputMap map_;
        QString path_;
        QStringList loadWarnings_;
        QUndoStack undo_;
    };

} // namespace imap
