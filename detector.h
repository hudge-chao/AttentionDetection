#ifndef DETECTOR_H
#define DETECTOR_H

#include <QThread>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <QDebug>
#include <vector>

class Detector: public QThread
{
    Q_OBJECT

public:
    enum DetectorStatus{
        Idle = 0,
        Detecting = 1,
        GetNewImage = 2
    };

public:
    Detector();
    void init();
    void run();
    DetectorStatus getStatus();
    void setImage(cv::Mat img);
    float sideFaceDetection(std::vector<cv::Point2f> &landmarks);
    bool eyesOpenDetection(cv::Mat imageFace);

private:
    cv::Mat image;
    float viewAngle;
    bool eyesOpenning;
    float ConcentrationCurrent;
    float ConcentrationAverage;
    DetectorStatus status;

signals:
    void sendConcentrationValues(float angle, bool eyesStatus, float current, float average);
    void warningForAttention(int type);
};

#endif // DETECTOR_H
