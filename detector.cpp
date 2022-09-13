#include "detector.h"

using namespace std;
using namespace cv;
using namespace cv::face;

Detector::Detector() : ConcentrationCurrent(1.0), ConcentrationAverage(1.0)
{
//    qDebug() << "Created Attention Detector";
//    qDebug() << "Attention Detecting";
    // TO DO:
}

void Detector::init(){
    status = DetectorStatus::Idle;
    start();
}

void Detector::run()
{
    // Load Face Detector
    CascadeClassifier faceDetector("haarcascade_frontalface_alt2.xml");
    //Load profile face Detector
    CascadeClassifier profileFaceDetector("haarcascade_profileface.xml");
    CascadeClassifier facesDetector("haarcascade_frontalface_alt_tree.xml");
    // Create an instance of Facemark
    Ptr<Facemark> facemark = FacemarkLBF::create();
    // Load landmark detector
    facemark->loadModel("lbfmodel.yaml");
    // Store former concentration
    float ConcentrationArray[5] = {1.0,1.0,1.0,1.0,1.0};
    bool eyesStatusArray[5] = {true,true,true,true,true};
    while(isRunning()){
        if(status == DetectorStatus::GetNewImage){
            status = DetectorStatus::Detecting;
            Mat imageGray;
            // face container
            vector<Rect> faces;
            // profileface container
            vector<Rect> profileFaces;
            vector<Rect> alfaces;
            //convert color image to gray image
            cvtColor(image, imageGray, COLOR_RGB2GRAY);
            // Detect upperbody
            profileFaceDetector.detectMultiScale(imageGray, profileFaces);
            facesDetector.detectMultiScale(imageGray, alfaces);
            // Detect faces
            faceDetector.detectMultiScale(imageGray, faces,1.1,3,0,Size(80,80));
            // Variable for landmarks.
            // Landmarks for one face is a vector of points
            vector<vector<Point2f> > landmarks;
            //fresh image forever one image per 10 ms until keyboard pressed
            // Run landmark detector
            static uint8_t cc = 0;
            static uint8_t nn = 0;
            bool success = facemark->fit(image,faces,landmarks);
            if(success){
                cc = 0;
                viewAngle = sideFaceDetection(landmarks[0]);
                eyesOpenning = eyesOpenDetection(imageGray(faces[0]));
                ConcentrationCurrent = (viewAngle*0.5) + (eyesOpenning*0.5);
                static uint8_t count = 0;
                ConcentrationArray[count] = ConcentrationCurrent;
                eyesStatusArray[count] = eyesOpenning;
                count == 4 ? count = 0 : count++;
                ConcentrationAverage = 0.0;
                bool alwaysClosed = true;
                for(int i = 0;i<5;i++){
                    ConcentrationAverage += ConcentrationArray[i];
                    alwaysClosed &= !eyesStatusArray[i];
                }
                ConcentrationAverage /= 5;
                if(alwaysClosed){
                    if(nn == 3){
                        nn = 0;
                        emit warningForAttention(1);
                    }
                    else{
                        nn++;
                    }
                }
                emit sendConcentrationValues(viewAngle,eyesOpenning,ConcentrationCurrent,ConcentrationAverage);
            }
            if(alfaces.size() == 0 && faces.size() == 0){
                if(cc == 8){
                    cc = 0;
                    emit warningForAttention(2);
                }
                else{
                    cc++;
                }
            }
            status = DetectorStatus::Idle;
        }
    }
}

Detector::DetectorStatus Detector::getStatus()
{
    return status;
}

void Detector::setImage(Mat img)
{
    image = img;
    status = DetectorStatus::GetNewImage;
}

float Detector::sideFaceDetection(vector<Point2f> &landmarks){
    uint16_t noseTip,faceRight,faceLeft;
    float ratio;
    noseTip = landmarks[30].x;
    faceRight = landmarks[1].x;
    faceLeft = landmarks[15].x;
    ratio = (float)(noseTip-faceRight)/(faceLeft-noseTip);
    if(ratio >= 1) ratio = (float)(1.00/ratio);
    return ratio;
}

bool Detector::eyesOpenDetection(Mat imageFace){
    //Load eyes Detector
    CascadeClassifier eyesDetector("haarcascade_eye.xml");
    // Find eyes
    vector<Rect> eyes;
    // Detect eyes
    eyesDetector.detectMultiScale(imageFace,eyes,1.1,3,0,Size(25,25),Size(50,50));
    if(eyes.size() > 0){
        Mat imageBinary,imageEye,imageLris;
        imageEye = imageFace(eyes[0]);
        adaptiveThreshold(imageEye,imageBinary,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY_INV,19,5);

        Mat element1 = getStructuringElement(MORPH_RECT,Size(4,4),Size(-1,-1));
        morphologyEx(imageBinary,imageBinary,MORPH_CLOSE,element1,Size(-1,-1),1);

        Mat element2 = getStructuringElement(MORPH_ELLIPSE,Size(9,9),Size(-1,-1));
        morphologyEx(imageBinary,imageLris,MORPH_OPEN,element2);
        return true;
    }
    else{
        return false;
    }
}

