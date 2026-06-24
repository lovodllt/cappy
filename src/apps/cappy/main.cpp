#include <QApplication>

#include "AppBootstrap.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Cappy");
    app.setOrganizationName("Cappy Project");
    app.setApplicationVersion("0.1.0");
    app.setQuitOnLastWindowClosed(false);

    AppBootstrap bootstrap(app);
    return bootstrap.run();
}
