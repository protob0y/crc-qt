#include "crc_main.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    crc_main w;
    w.show(); // Zeigt das Fenster an
    return a.exec();
}
