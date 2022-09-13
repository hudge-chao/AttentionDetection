#include "mainwindow.h"
#include <QApplication>
#include "login.h"
#include <QString>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Login login;
    if(login.exec() == QDialog::Accepted){
        MainWindow w;
        w.showFullScreen();
        return a.exec();
    }

}
