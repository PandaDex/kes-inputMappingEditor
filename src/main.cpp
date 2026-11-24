#include "MainWindow.h"
#include "KesStyle.h"

#include <QApplication>

int main(int argc, char** argv)
{
    // Qt's native Windows file dialogs keep the main thread blocked for seconds after closing.
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("kes"));
    QApplication::setApplicationName(QStringLiteral("InputMapEditor"));

    kes::style::apply(app);

    imap::MainWindow window;
    const QStringList args = app.arguments();
    if (args.size() > 1) window.openFile(args.at(1));
    window.show();
    return app.exec();
}
