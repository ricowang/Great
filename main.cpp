#include "greatwindow.h"
#include <QApplication>
#include "viewmanager.h"
#include "downloadmanager.h"
#include "contentmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ViewMgr;
    DownloadMgr;
    ContentMgr;

    ViewMgr.start();
    ViewMgr.show();

    return a.exec();
}
