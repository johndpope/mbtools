#include "MapView.h"


#include <QtGui>
#include <QApplication>

#include "MapView.h"

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    MapView mainWindow;

    mainWindow.show();

    return app.exec();
}
