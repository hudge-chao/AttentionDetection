#include "login.h"
#include "ui_login.h"
#include <QLineEdit>
#include <QSettings>
#include <QMessageBox>

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    ui->pwd->setEchoMode(QLineEdit::Password);
}

Login::~Login()
{
    delete ui;
}

QString Login::getUserName(){
    return userName;
}

void Login::on_loginButton_clicked()
{
    inputPwd = ui->pwd->text();
    userName = ui->username->text();
    QSettings account(QString("settings.ini"),QSettings::IniFormat);
    account.setIniCodec("UTF-8");
    defaultPwd = account.value("student/password",0).toString();
    defaultName = account.value("student/account",0).toString();
    if(inputPwd == defaultPwd && userName == defaultName){
        accept();
    }
    if(inputPwd != defaultPwd && userName != defaultName){
        QMessageBox::warning(this, QString("登录失败"), QString("用户名或者密码错误！"),QMessageBox::StandardButton::Close);
    }
}
