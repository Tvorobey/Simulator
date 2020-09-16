//Qt
#include <QCoreApplication>

//SDK
#include <CMainHandler/CMainHandler.h>

//Local
#include "AppController.h"

int main(int argc, char *argv[])
{
    int retCode = 0;
    QCoreApplication app{ argc, argv };

    CMainHandler::setApplicationVersion( APP_VERSION, APP_REVISION );
    retCode = CMainHandler::parseArgs();
    if(retCode != 0)
    {
        return retCode;
    }
    return CMainHandler::runController( new AppController{ & app } );
}
