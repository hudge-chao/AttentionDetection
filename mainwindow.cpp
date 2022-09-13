#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPixmap>
#include <QList>
#include <QIODevice>
#include <QTextCodec>
#include <QSettings>

using namespace cv;
QElapsedTimer t;
float allAvgAttention[10] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
int idxOfAttention = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->angleBox->setReadOnly(true);
    ui->eyesBox->setReadOnly(true);
    ui->currentBox->setReadOnly(true);
    ui->averageBox->setReadOnly(true);
    ui->angleBox->setText("0.0");
    ui->eyesBox->setText("OPEN");
    ui->currentBox->setText("1.0");
    ui->averageBox->setText("0.0");
    QSettings set(QString("settings.ini"),QSettings::IniFormat);
    set.setIniCodec("UTF-8");
    //QTextCodec* utfCodec = QTextCodec::codecForName("utf-8");
//    set.setValue("student/name","李玟");
//    set.setValue("student/grade","八年级");
//    set.setValue("student/gender","男");
//    set.setValue("student/class","数学");
//    QByteArray strZhArray = set.value("student/name",0).toByteArray();
//    QString uniCodeArray = utfCodec->toUnicode(strZhArray);
//    QString strZh = set.value("student/grade",0);
//    qDebug() << uniCodeArray;
    ui->stuName->setText(set.value("student/name",0).toString());
    ui->grade->setText(set.value("student/grade",0).toString());
    ui->xuehao->setText(set.value("student/id",0).toString());
    ui->gender->setText(set.value("student/gender",0).toString());
    ui->curClass->setText(set.value("student/class",0).toString());
    timer = new QTimer(this);
    detector.init();
    RGBCamera.open(1);
    RGBCamera.set(CAP_PROP_FRAME_WIDTH,640);
    RGBCamera.set(CAP_PROP_FRAME_HEIGHT,480);
    RGBCamera.set(CAP_PROP_FPS,30);
    connect(timer,SIGNAL(timeout()),this,SLOT(showImage()));
    connect(&detector,SIGNAL(sendConcentrationValues(float,bool,float,float)),this,SLOT(getConcentrationValues(float,bool,float,float)));
    connect(&detector,SIGNAL(warningForAttention(int)),this,SLOT(warningFocus(int)));
    timer->start(1000/30);
    QList<QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();
    ttsPortInit();
    dtuPortInit();
    alarm(QString("开始上课"));
}

MainWindow::~MainWindow()

{
    delete ui;
}

void MainWindow::showImage()
{
    if(RGBCamera.isOpened()){
        Mat ImageRGB,img;
        RGBCamera >> ImageRGB;
        if(detector.getStatus() == Detector::DetectorStatus::Idle){
            detector.setImage(ImageRGB);
        }
        cvtColor(ImageRGB,img,COLOR_RGB2BGR,3);
        Image = QImage((const unsigned char*)(img.data),img.cols,img.rows,img.step[0],QImage::Format_RGB888);
        ui->rgbCamera->setPixmap(QPixmap::fromImage(Image));
    }
    else{
        try{
            RGBCamera.open(1);
            RGBCamera.set(CAP_PROP_FRAME_WIDTH,640);
            RGBCamera.set(CAP_PROP_FRAME_HEIGHT,480);
            RGBCamera.set(CAP_PROP_FPS,30);
        }
        catch(cv::Exception &e){
            qDebug() << e.what();
        }
    }
}

void MainWindow::sendMessage(QString message)
{
    serialWrite(serialDTU, message);
}

void MainWindow::alarm(QString text)
{
    serialWrite(serialTTS, text);
}

void MainWindow::getConcentrationValues(float angle, bool eyesStatus, float ConcentrationCurrent, float ConcentrationAverage)
{
    //qDebug() << angle << " " << eyesStatus << " " << ConcentrationCurrent << " " << ConcentrationAverage;
    ui->angleBox->setText(QString::number(angle));
    QString eyesOpenStr;
    eyesStatus == true ? eyesOpenStr.append("OPEN") : eyesOpenStr.append("CLOSE");
    ui->eyesBox->setText(eyesOpenStr);
    ui->currentBox->setText(QString::number(ConcentrationCurrent));
    ui->averageBox->setText(QString::number(ConcentrationAverage));
    allAvgAttention[idxOfAttention] = ConcentrationAverage;
    idxOfAttention == 9 ? 0 : idxOfAttention++;
}

void MainWindow::warningFocus(int type)
{
    if(type == 1){
        alarm(QString("请目视前方"));
        return;
    }
    if(type == 2){
        alarm(QString("坐姿不正或者离开座位"));
        return;
    }
}

void MainWindow::serialDataCallback()
{
     QByteArray buf;
     buf = serialDTU->readAll();
     if (!buf.isEmpty())
     {
        QString str = "";
        str += tr(buf);
        qDebug() << str;
     }
}

void MainWindow::setPhoneNumber(QString number)
{
    PhoneNumber = number;
}

void MainWindow::serialWrite(QSerialPort *serialPort, QString Msg)
{
//    if(serialPort->isOpen()){
//        serialPort->close();
//    }
//    if(serialPort->open(QSerialPort::ReadWrite)){
        //serialPort->clear(QSerialPort::AllDirections);
        QTextCodec* gbkCodec = QTextCodec::codecForName("gbk");
        QByteArray ByteGb2312 = gbkCodec->fromUnicode(Msg);
        serialPort->write(ByteGb2312.data());
        serialPort->waitForBytesWritten(50);
//        serialPort->close();
//    }
}

void MainWindow::ttsPortInit()
{
    serialTTS = new QSerialPort("TTS");
    serialTTS->setBaudRate(QSerialPort::Baud9600);
    serialTTS->setDataBits(QSerialPort::Data8);
    serialTTS->setParity(QSerialPort::NoParity);
    serialTTS->setStopBits(QSerialPort::OneStop);
    serialTTS->setFlowControl(QSerialPort::NoFlowControl);
    serialTTS->open(QSerialPort::ReadWrite);
//    if(serialTTS->open(QIODevice::WriteOnly)){
//        qDebug() << "open port success";
//        serialTTS->clear(QSerialPort::AllDirections);
//        serialTTS->close();
//    }
    //connect(serialTTS, SIGNAL(readyRead()), this, SLOT(serialDataCallback(QSerialPort*)));
}

void MainWindow::dtuPortInit()
{
    serialDTU = new QSerialPort("DTU");
    serialDTU->setBaudRate(QSerialPort::Baud115200);
    serialDTU->setDataBits(QSerialPort::Data8);
    serialDTU->setParity(QSerialPort::NoParity);
    serialDTU->setStopBits(QSerialPort::OneStop);
    serialDTU->setFlowControl(QSerialPort::NoFlowControl);
    serialDTU->open(QSerialPort::ReadWrite);
    connect(serialDTU, SIGNAL(readyRead()), this, SLOT(serialDataCallback()));
    sendMessage("AT+ENTM\r\n");
    delay(100);
    sendMessage(QString("+++"));
    delay(100);
    sendMessage(QString("a"));
    delay(100);
}

void MainWindow::delay(uint16_t mDelay)
{
    t.restart();
    while(t.elapsed() < mDelay);
}

void MainWindow::on_pushButton_clicked()
{

    QSettings ini(QString("settings.ini"),QSettings::IniFormat);
    float AvgAttention = 0.0;
    for(int n = 0; n < 10; n++){
        AvgAttention += allAvgAttention[n];
    }
    AvgAttention /= 10.0;
    QString strForSend("AT+SMSEND=");
    QString PhoneNumber = ini.value("teacher/number",0).toString();
    strForSend.append(PhoneNumber);
    strForSend.append(",");
    //QString Msg("test");
    strForSend.append("尊敬的学生家长，您好！您的孩子");
    strForSend.append(ui->stuName->text());
    strForSend.append("在"+ui->curClass->text()+"上平均专注度为");
    //strForSend.append(QString::number(1.00));
    strForSend.append(QString::number(AvgAttention));
    strForSend.append("\r\n");
    alarm(QString("上课结束"));
    //qDebug() << strForSend;
    sendMessage(strForSend);
    delay(100);
    //sendMessage("AT+CSQ\r\n");
    qApp->quit();
}
