#include "DisKatalog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args = QApplication::arguments();
    DisKatalog disKatalog(args);
    disKatalog.showMaximized();
    return app.exec();
}

