#include <QApplication>
#include "window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("EMT S2P card Demo");
    app.setOrganizationName("e-Machine Techology");
    QFont f = app.font();
    f.setFamily("Monaco");
    f.setPointSize(32);
    f.setBold(true);
    app.setFont(f);
   // app.setStyle("windows");
    app.setStyle("plastique");

    Window window;
#ifdef Q_OS_SYMBIAN
    window.showMaximized();
#else
    window.showFullScreen();
    window.show();
#endif
    return app.exec();
}
