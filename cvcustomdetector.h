/*
 * this includes several important class
 * that are used for face detection and inferencing
*/




#ifndef CVCUSTOMDETECTOR_H
#define CVCUSTOMDETECTOR_H

#include <iostream>
#include <fstream>

#include <QObject>
// #include <QAbstractVideoFilter>
#include <QDebug>
#include <QTemporaryFile>
#include <QMutex>

#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QVideoSink>
#include <QVideoFrame>
#include <QtWebSockets/QtWebSockets>


#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/ocl.hpp>

// #ifdef Q_OS_ANDROID

#include <ncnn/mat.h>
#include <ncnn/net.h>
#include <ncnn/gpu.h>
#include <ncnn/command.h>
#include <ncnn/pipeline.h>
#include "ncnn/benchmark.h"
// #endif

using namespace cv;
using namespace cv::dnn;
using namespace std;


// detection
class YuNet
{
public:
    YuNet(){}
    YuNet(const std::string& model_path,
          const cv::Size& input_size = cv::Size(320, 320),
          float conf_threshold = 0.9f,
          float nms_threshold = 0.3f,
          int top_k = 5000,
          int backend_id = 0,
          int target_id = 0)
        : model_path_(model_path), input_size_(input_size),
        conf_threshold_(conf_threshold), nms_threshold_(nms_threshold),
        top_k_(top_k), backend_id_(backend_id), target_id_(target_id)
    {

        createModel();
    }

    YuNet(std::vector<uchar> model_buffer,
          const cv::Size& input_size = cv::Size(300, 300),
          float conf_threshold = 0.9f,
          float nms_threshold = 0.3f,
          int top_k = 5000,
          int backend_id = 0,
          int target_id = 0)
        : model_buffer_(model_buffer), input_size_(input_size),
        conf_threshold_(conf_threshold), nms_threshold_(nms_threshold),
        top_k_(top_k), backend_id_(backend_id), target_id_(target_id)
    {

        createModel();
    }


    void createModel(){
        model = cv::FaceDetectorYN::create(model_path_, "",
                                           input_size_,
                                           conf_threshold_,
                                           nms_threshold_,
                                           top_k_,
                                           backend_id_,
                                           target_id_);
    }

    bool empty(){
        return model.empty();
    }

    void setPreferableTarget(cv::dnn::Target targetId){
        target_id_= targetId;
        if (!model.empty())
            createModel();
    }

    void setPreferableBackend(cv::dnn::Backend backendId){
        backend_id_ = backendId;
        if (!model.empty())
            createModel();

    }

    /* Overwrite the input size when creating the model. Size format: [Width, Height].
    */
    void setInputSize(const cv::Size& input_size)
    {
        if (input_size == input_size_)
            return;

        input_size_ = input_size;

        if (!model.empty()){
            model->setInputSize(input_size_);
            return;
        }
        createModel();
    }

    cv::Mat infer(const cv::Mat image)
    {
        cv::Mat res;
        if (!model.empty())
            model->detect(image, res);
        return res;
    }



private:
    cv::Ptr<cv::FaceDetectorYN> model;

    std::string model_path_;
    std::vector<uchar> model_buffer_;
    cv::Size input_size_;
    float conf_threshold_;
    float nms_threshold_;
    int top_k_;
    int backend_id_;
    int target_id_;
};


// inferencing
class CVcustomDetector: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int counter READ getCounter WRITE setCounter RESET resetCounter NOTIFY counterChanged FINAL)
    Q_PROPERTY(QVideoSink* videoSink READ getVideoSink WRITE setVideoSink FINAL)

public:
    CVcustomDetector();
    ~CVcustomDetector();


    int counter=0;
    bool isProcessing=false;
    int getCounter() const;
    void setCounter(int newCounter);
    void resetCounter();
    QFuture<void> processingThread;
    QFuture<void> preprocessingThread;


    QVideoSink* videoSink = nullptr;
    void setVideoSink(QVideoSink* sink);
    QVideoSink* getVideoSink();

    int padding = 5;
    double conf_threshold = 0.9;
    Scalar faceMeanVal = Scalar(104, 117, 123);
    const float NcnnMean_face_vals[3] = {104.f, 117.f, 123.f};

    ncnn::Net ncnnAgeNet;
    ncnn::Net ncnnGenderNet;



    Scalar MODEL_MEAN_VALUES = Scalar(78.4263377603, 87.7689143744, 114.895847746);
    const float NcnnMean_model_vals[3] = {78.4263377603, 87.7689143744, 114.895847746};

    QVector<QString> ageList = {"(0-2)", "(4-6)", "(8-12)", "(15-20)", "(25-32)",
                                "(38-43)", "(48-53)", "(60-100)"};
    QVector<QString> genderList = {"Male", "Female"};

    QVector<cv::Rect>  getFaceBox_yunet(Mat* frame);
    QVector<cv::Rect>  getFaceBox_classifier(cv::Mat* frame);



    cv::Mat convertToOCVMat(QVideoFrame *input);


//  #ifdef Q_OS_ANDROID
    bool NCNN_USE_VULKAN=false;
    ncnn::Mat convertToNcnnMat(QVideoFrame *input);
    void detect_NCNN(ncnn::Mat ncMat);
#ifdef Q_OS_ANDROID
    void detect_OCV_RKNN(cv::Mat mat);
#endif
    bool rk= true;
//  #endif

public slots:
    void run(QVideoFrame *input);

signals:
    void objectDetected(float x, float y, float w, float h);
    void objectsDetected(QString faceData);
    void noObject(void);

    void counterChanged();

};

#endif // CVCUSTOMDETECTOR_H
