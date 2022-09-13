#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <QTimer>
#include "detector.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QElapsedTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setPhoneNumber(QString number);


private:
    Ui::MainWindow *ui;
    QImage Image;
    cv::VideoCapture RGBCamera;
    QString PhoneNumber;
    QTimer *timer;
    Detector detector;
    QSerialPort *serialDTU;
    QSerialPort *serialTTS;
    void serialWrite(QSerialPort *serialPort, QString Msg);
    void ttsPortInit();
    void dtuPortInit();
    void delay(uint16_t mDelay);

public slots:
    void showImage();
    void sendMessage(QString message);
    void alarm(QString text);
    void getConcentrationValues(float angle, bool eyesStatus, float ConcentrationCurrent, float ConcentrationAverage);
    void warningFocus(int type);
    void serialDataCallback();
private slots:
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
