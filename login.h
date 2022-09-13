#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QString>
#include <QDebug>


namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    QString getUserName();

private slots:
    void on_loginButton_clicked();

private:
    Ui::Login *ui;
    QString userName;
    QString defaultPwd;
    QString defaultName;
    QString inputPwd;
};

#endif // LOGIN_H
