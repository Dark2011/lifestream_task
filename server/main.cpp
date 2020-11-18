#include <QtCore/QCoreApplication>
#include "driver.h"


int main(int argc, char *argv[])

{
    QCoreApplication a(argc, argv);

    int port = 7755; // Default port
    if (argc > 1)
        port = atoi(argv[1]);
    
    ServerDriver driver(port);


    return a.exec();
}
