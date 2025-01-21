/*
 * this is CVcustomDetector class impelementation
*/

#include "cvcustomdetector.h"


YuNet yuNet;
cv::CascadeClassifier classifier;



#ifdef Q_OS_ANDROID

#include "rknn.h"
RKNN  rk_age;
RKNN  rk_gender;

#endif

CVcustomDetector::CVcustomDetector(){

    cv::setNumThreads(2);
    cv::setUseOptimized(true);


    ncnn::create_gpu_instance();

    if (ncnn::get_gpu_count() != 0)
    {
        qDebug() << "atleast one GPU found, using Vulkan then.";
        NCNN_USE_VULKAN= true;
    }else {
        qDebug() << "No VK GPU found, skip using Vulkan then.";
    }

    // load models
    if(yuNet.empty())
    {
        QFile yunetModel (":/models/face_detection_yunet_2023mar.onnx");


        if(yunetModel.open(QFile::ReadOnly))
        {
            QFile f= QFile(QDir::tempPath()+"/face_detection_yunet_2023mar.onnx");

            if(f.open(QIODeviceBase::ReadWrite))
            {

                f.write(yunetModel.readAll());
                f.close();
                qDebug() << "file name:"<<f.fileName().toStdString();

                {

                    yuNet = YuNet(f.fileName().toStdString(),
                                  cv::Size(300, 300),
                                  0.9f,
                                  0.3f,
                                  5000,
                                  0,
                                  0);
                }
                if(!yuNet.empty())
                    qDebug() << "yuNet loaded successfully.";
                else
                    qDebug() << "failed to loaded yuNet.";

            }
            else
            {
                qDebug() << "Can't open model temp file.";
            }
        }
        else
        {
            qDebug() << "Can't open yuNet Model.";
        }
    }


    {
        QFile gendernetModel (":/models/ncnn/gender_googlenet.bin");
        QFile gendernetConfig(":/models/ncnn/gender_googlenet.param");
        if(gendernetModel.open(QFile::ReadOnly) && gendernetConfig.open(QFile::ReadOnly))
        {
            QTemporaryFile model;
            QTemporaryFile config;
            if(model.open())
            {
                model.write(gendernetModel.readAll());
                model.close();


                if(config.open())
                {
                    config.write(gendernetConfig.readAll());
                    config.close();

                    if (NCNN_USE_VULKAN){
                        ncnnGenderNet.opt.use_vulkan_compute = NCNN_USE_VULKAN;
                        ncnnGenderNet.set_vulkan_device(0);
                        ncnn::VkAllocator* blob_vkallocator = ncnn::get_gpu_device(0)->acquire_blob_allocator();
                        ncnn::VkAllocator* staging_vkallocator = ncnn::get_gpu_device(0)->acquire_blob_allocator();
                        ncnnGenderNet.opt.blob_vkallocator = blob_vkallocator;
                        ncnnGenderNet.opt.workspace_vkallocator = blob_vkallocator;
                        ncnnGenderNet.opt.staging_vkallocator = staging_vkallocator;

                        ncnnGenderNet.opt.use_shader_local_memory= true;

                        ncnnGenderNet.opt.use_fp16_packed = false;
                        ncnnGenderNet.opt.use_fp16_storage = false;
                        ncnnGenderNet.opt.use_fp16_arithmetic = false;
                        ncnnGenderNet.opt.use_int8_storage = false;
                        ncnnGenderNet.opt.use_int8_arithmetic = false;
                    }
                    // ncnnGenderNet.opt.num_threads = 1;
                    ncnnGenderNet.opt.use_local_pool_allocator= true;
                    // ncnnGenderNet.opt.lightmode =true;
                    ncnnGenderNet.opt.use_winograd_convolution =false;
                    // ncnnGenderNet.opt.use_winograd23_convolution=true;
                    // ncnnGenderNet.opt.use_winograd43_convolution=true;
                    // ncnnGenderNet.opt.use_winograd63_convolution=true;

                    ncnnGenderNet.opt.use_sgemm_convolution =true;

#ifdef Q_OS_ANDROID
                    ncnnGenderNet.opt.use_packing_layout = true;
                    ncnnGenderNet.opt.use_shader_pack8=false;
#endif

                    // load param before model
                    auto load_param_ret = ncnnGenderNet.load_param(config.fileName().toStdString().c_str());
                    auto load_model_ret = ncnnGenderNet.load_model(model.fileName().toStdString().c_str());
                    if(!load_model_ret && ! load_param_ret)
                        qDebug() << "ncnnGenderNet loaded successfully.";
                    else
                        qDebug() << "failed to loaded ncnnGenderNet:"<< load_model_ret <<load_param_ret;

                }
                else
                {
                    qDebug() << "Can't open config temp file.";
                }
            }
            else
            {
                qDebug() << "Can't open model temp file.";
            }
        }
        else
        {
            qDebug() << "Can't open ncnnGenderNet Model.";
        }
    }


    {
        QFile agenetModel (":/models/ncnn/age_googlenet.bin");
        QFile agenetConfig(":/models/ncnn/age_googlenet.param");
        if(agenetModel.open(QFile::ReadOnly) && agenetConfig.open(QFile::ReadOnly))
        {
            QTemporaryFile model;
            QTemporaryFile config;
            if(model.open())
            {
                model.write(agenetModel.readAll());
                model.close();

                if(config.open())
                {
                    config.write(agenetConfig.readAll());
                    config.close();

                    if (NCNN_USE_VULKAN){
                        ncnnAgeNet.opt.use_vulkan_compute = NCNN_USE_VULKAN;
                        ncnnAgeNet.set_vulkan_device(0);
                        ncnn::VkAllocator* blob_vkallocator = ncnn::get_gpu_device(0)->acquire_blob_allocator();
                        ncnn::VkAllocator* staging_vkallocator = ncnn::get_gpu_device(0)->acquire_blob_allocator();
                        ncnnAgeNet.opt.blob_vkallocator = blob_vkallocator;
                        ncnnAgeNet.opt.workspace_vkallocator = blob_vkallocator;
                        ncnnAgeNet.opt.staging_vkallocator = staging_vkallocator;

                        ncnnAgeNet.opt.use_shader_local_memory= true;

                        ncnnAgeNet.opt.use_fp16_packed = false;
                        ncnnAgeNet.opt.use_fp16_storage = false;
                        ncnnAgeNet.opt.use_fp16_arithmetic = false;
                        ncnnAgeNet.opt.use_int8_storage = false;
                        ncnnAgeNet.opt.use_int8_arithmetic = false;
                    }
                    // ncnnAgeNet.opt.num_threads = 3;
                    // ncnnAgeNet.opt.lightmode =true;
                    ncnnAgeNet.opt.use_local_pool_allocator =true;
                    ncnnAgeNet.opt.use_winograd_convolution =false;
                    // ncnnAgeNet.opt.use_winograd23_convolution=true;
                    // ncnnAgeNet.opt.use_winograd43_convolution=true;
                    // ncnnAgeNet.opt.use_winograd63_convolution=true;

                    ncnnAgeNet.opt.use_sgemm_convolution =true;

#ifdef Q_OS_ANDROID
                    ncnnAgeNet.opt.use_packing_layout = true;
                    ncnnAgeNet.opt.use_shader_pack8=false;
#endif


                    // load param before model
                    auto load_param_ret = ncnnAgeNet.load_param(config.fileName().toStdString().c_str());
                    auto load_model_ret = ncnnAgeNet.load_model(model.fileName().toStdString().c_str());
                    if(!load_model_ret && ! load_param_ret)
                        qDebug() << "ncnnAgeNet loaded successfully.";
                    else
                        qDebug() << "failed to loaded ncnnAgeNet." << load_model_ret << load_param_ret;
                }
                else
                {
                    qDebug() << "Can't open config temp file.";
                }
            }
            else
            {
                qDebug() << "Can't open model temp file.";
            }
        }
        else
        {
            qDebug() << "Can't open ncnnAgeNet Model.";
        }
    }

#ifdef Q_OS_ANDROID
    if(rk) {

    /*
     * Age model attributes
     * model input num:  1 , output num:  1
     * input  : index= 0 name= input n_dims= 4 dims= 1 224 224 3 n_elems= 150528 size= 301056 fmt= NHWC type= FP16 qnt_type= AFFINE zp= 0 scale= 1
     * output : index= 0 name= loss3/loss3_Y n_dims= 2 dims= 1 8 0 0 n_elems= 8 size= 16 fmt= UNDEFINED type= FP16 qnt_type= AFFINE zp= 0 scale= 1    */

        if (!rk_age.is_inited())
        {
            QFile rknn_age(":/models/rknn/rk3566/age_googlenet.rknn");

            if (rknn_age.open(QFile::ReadOnly))
            {
                QFile f= QFile(QDir::tempPath()+"/age_googlenet.rknn");
                if(f.open(QIODeviceBase::ReadWrite))
                {
                    f.write(rknn_age.readAll());
                    f.close();
                    qDebug() << "file name:"<<f.fileName().toStdString();


                    rk_age.load_model(f.fileName().toStdString());


                    if(rk_age.init_model())
                        qDebug() << "failed to loaded rknn.";
                    else
                        qDebug() << "rknn loaded successfully.";
                }
                else
                {
                    qDebug() << "Can't open model temp file.";
                }
            }
            else
            {
                qDebug() << "Can't open rknn Model.";
            }
        }


        /*
     * Gender model attributes
     * model input num:  1 , output num:  1
     * input  : index= 0 name= input n_dims= 4 dims= 1 224 224 3 n_elems= 150528 size= 301056 fmt= NHWC type= FP16 qnt_type= AFFINE zp= 0 scale= 1
     * output : index= 0 name= loss3/loss3_Y n_dims= 2 dims= 1 2 0 0 n_elems= 2 size= 4 fmt= UNDEFINED type= FP16 qnt_type= AFFINE zp= 0 scale= 1
    */
        if (!rk_gender.is_inited())
        {
            QFile rknn_age(":/models/rknn/rk3566/gender_googlenet.rknn");

            if (rknn_age.open(QFile::ReadOnly))
            {
                QFile f= QFile(QDir::tempPath()+"/gender_googlenet.rknn");
                if(f.open(QIODeviceBase::ReadWrite))
                {
                    f.write(rknn_age.readAll());
                    f.close();
                    qDebug() << "file name:"<<f.fileName().toStdString();


                    rk_gender.load_model(f.fileName().toStdString());


                    if(rk_gender.init_model())
                        qDebug() << "failed to loaded rknn.";
                    else
                        qDebug() << "rknn loaded successfully.";
                }
                else
                {
                    qDebug() << "Can't open model temp file.";
                }
            }
            else
            {
                qDebug() << "Can't open rknn Model.";
            }
        }
    }
#endif

}

CVcustomDetector::~CVcustomDetector()
{
    if(!processingThread.isFinished()){
        processingThread.cancel();
        processingThread.waitForFinished();
    }

    // if (NCNN_USE_VULKAN){
    //     ncnn::get_gpu_device(0)->reclaim_blob_allocator(ncnnAgeNet.opt.blob_vkallocator);
    //     ncnn::get_gpu_device(0)->reclaim_blob_allocator(ncnnAgeNet.opt.staging_vkallocator);

    //     ncnn::get_gpu_device(0)->reclaim_blob_allocator(ncnnGenderNet.opt.blob_vkallocator);
    //     ncnn::get_gpu_device(0)->reclaim_blob_allocator(ncnnGenderNet.opt.staging_vkallocator);
    //     ncnn::destroy_gpu_instance();
    // }

#ifdef Q_OS_ANDROID
    if (rk_age.is_inited())
        rk_age.release_model();

    if (!rk_age.is_inited())
        rk_gender.release_model();
#endif
}

int CVcustomDetector::getCounter() const
{
    return counter;
}

void CVcustomDetector::setCounter(int newCounter)
{
    if (counter == newCounter)
        return;
    if (newCounter == 0 ){
        counter = newCounter;
        emit noObject();
        return;
    }
    counter = newCounter;
    emit counterChanged();
}

void CVcustomDetector::resetCounter()
{
    setCounter({0}); // TODO: Adapt to use your actual default value
}

void CVcustomDetector::setVideoSink(QVideoSink* sink)
{
    videoSink = sink;
    connect(this->videoSink, &QVideoSink::videoFrameChanged, this,[this](const  QVideoFrame &frame){
        run(const_cast<QVideoFrame*>(&frame));
    });
}

QVideoSink *CVcustomDetector::getVideoSink()
{
    return videoSink;
}

QVector<cv::Rect>  CVcustomDetector::getFaceBox_yunet(Mat* frame)
{

    cv::Mat faces;
    yuNet.setInputSize(cv::Size(frame->cols,frame->rows));

    faces = yuNet.infer(*frame);
    // qDebug() << "getFaceBox_yunet rows:"<<  faces.rows;

    QVector<cv::Rect> bboxes;
    for (int i = 0; i < faces.rows; ++i)
    {
        // Detect bounding boxes
        int x0 = static_cast<int>(faces.at<float>(i, 0));
        int y0 = static_cast<int>(faces.at<float>(i, 1));
        int w0 = static_cast<int>(faces.at<float>(i, 2));
        int h0 = static_cast<int>(faces.at<float>(i, 3));


        // qDebug() <<  "yunet: "<< x0<<y0<<w0<<h0;
        int x,y,w,h;
        x = (x0 - padding) > 0 ?
                ((x0 - padding) < frame->cols ? (x0 - padding) : frame->cols)
                               : 0;
        y = (y0 - padding) > 0 ?
                ((y0 - padding) < frame->rows ? (y0 - padding) : frame->rows)
                               : 0;
        w = (x0+ w0 + 2*padding) > 0 ?
                ((x0+ w0 + 2*padding) < frame->cols ? (w0 + 2*padding) : (frame->cols - x0 ))
                                        : 0;
        h = (y0 + h0 + 2*padding) > 0 ?
                ((y0 + h0 + 2*padding) < frame->rows ? (h0 + 2*padding) : (frame->rows - y0 ))
                                        : 0;

        cv::Rect box( x, y, w, h );
        // qDebug() <<  "yunet after proc: "<< x<<y<<w<<h;
        bboxes.push_back(box);
    }
    return bboxes;
}

QVector<Rect> CVcustomDetector::getFaceBox_classifier(Mat* frame)
{
    Mat frameGray;

    if(frame->channels() == 3){
        cvtColor( *frame, frameGray, COLOR_BGR2GRAY );
        //cv::ocl::finish()
    }else if(frame->channels() == 4) {
        cvtColor( *frame, frameGray, COLOR_BGRA2GRAY );
        //cv::ocl::finish()
    }

    equalizeHist( frameGray, frameGray );

    std::vector<cv::Rect> detected;
    classifier.detectMultiScale(frameGray, detected, 1.1);

    return QVector<cv::Rect>(detected.begin(),detected.end());
}


cv::Mat CVcustomDetector::convertToOCVMat(QVideoFrame *input)
{

    QImage image;
    image = input->toImage();

    if(image.width()>640 || image.height()> 480)
        image = image.scaled(QSize(640,480),Qt::KeepAspectRatio,Qt::FastTransformation);

    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.rgbSwapped();

    cv::Mat out(image.height(),
                image.width(),
                CV_8UC3,
                image.bits(),
                image.bytesPerLine());


    return out;
}


void CVcustomDetector::run(QVideoFrame* input)
{
    if(!input || !input->isValid()){
        return ;
    }

    if(isProcessing){
        return ;
    }

    if(!processingThread.isFinished()){
        return ;
    }

    if(input->map(QVideoFrame::ReadOnly)){

#ifdef Q_OS_ANDROID

        if (rk)
            processingThread = QtConcurrent::run(&CVcustomDetector::detect_OCV_RKNN, this, convertToOCVMat(input));
        else
            processingThread = QtConcurrent::run(&CVcustomDetector::detect_NCNN, this, convertToNcnnMat(input));

#else

        processingThread = QtConcurrent::run(&CVcustomDetector::detect_NCNN, this, convertToNcnnMat(input));
#endif
        input->unmap();
    }

    return;
}


// #ifdef Q_OS_ANDROID

ncnn::Mat CVcustomDetector::convertToNcnnMat(QVideoFrame *input)
{

    QImage image;
    image = input->toImage();


    if(image.width()>640 || image.height()> 480)
        image = image.scaled(QSize(640,480),Qt::KeepAspectRatio,Qt::FastTransformation);

    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.rgbSwapped();
    ncnn::Mat blob;

    return  blob.from_pixels(image.bits(), ncnn::Mat::PIXEL_BGR, image.width() , image.height());
}

void CVcustomDetector::detect_NCNN(ncnn::Mat ncMat){
    isProcessing= true;

    cv::Mat cvMat(ncMat.h,ncMat.w,CV_8UC3);
    ncMat.to_pixels(cvMat.data, ncnn::Mat::PIXEL_BGR);

    QVector<cv::Rect> bboxes = getFaceBox_yunet(&cvMat);

    if (!ncMat.empty() && !cvMat.empty()){

        QJsonObject detection;
        detection.insert("total_count",bboxes.size());

        int M=0,F=0;

        if(bboxes.size()==0){
            emit noObject();
        }
        else{

            QJsonArray faces;


            // cv::parallel_for_(cv::Range(0,bboxes.size()), [&](const cv::Range& range){ // parallel_for_
            // for (auto it = /*0*/ range.start; it < /*bboxes.size()*/ range.end; it++) { //for loop
            for (auto it = 0 ; it < bboxes.size(); it++) { //for loop


                // take the ROI of box on the frame
                Rect rec = bboxes.at(it);
                cv::Mat faceRect = cvMat(rec).clone();
                ncnn::Mat ncRioRect = ncnn::Mat::from_pixels_resize(faceRect.data,
                                                                    ncnn::Mat::PIXEL_BGR,
                                                                    faceRect.cols,
                                                                    faceRect.rows,
                                                                    224,224);

                // apply mean values
                ncRioRect.substract_mean_normalize(NcnnMean_model_vals,0);


                // using ncnn
                ncnn::Mat ncGenderOut;
                ncnn::Mat ncAgeOut;
                ncnn::Extractor  ncGenderEx = ncnnGenderNet.create_extractor();
                ncnn::Extractor  ncAgeEx    = ncnnAgeNet.create_extractor();


                ncGenderEx.input("input",ncRioRect);
                ncGenderEx.extract("loss3/loss3_Y",ncGenderOut);
                auto ncGenderOutFlat = ncGenderOut.reshape(ncGenderOut.w * ncGenderOut.h * ncGenderOut.c);

                std::vector<float> genderPreds;
                genderPreds.resize(ncGenderOutFlat.w);
                for (int j=0; j<ncGenderOutFlat.w; j++)
                {
                    genderPreds[j] = ncGenderOutFlat[j];
                }

                QString gender ;
                int max_index_gender = std::distance(genderPreds.begin(), max_element(genderPreds.begin(), genderPreds.end()));
                if (genderPreds[max_index_gender]> 0.9)
                    gender = genderList[max_index_gender];
                else
                    continue;


                ncAgeEx.input("input",ncRioRect);
                ncAgeEx.extract("loss3/loss3_Y",ncAgeOut);
                auto ncAgeOutFlat    = ncAgeOut.reshape(ncAgeOut.w * ncAgeOut.h * ncAgeOut.c);

                std::vector<float> agePreds;
                agePreds.resize(ncAgeOutFlat.w);
                for (int j=0; j<ncAgeOutFlat.w; j++)
                {
                    agePreds[j] = ncAgeOutFlat[j];
                }
                int max_indice_age = std::distance(agePreds.begin(), max_element(agePreds.begin(), agePreds.end()));
                QString age = ageList[max_indice_age];

                gender == "Male" ? M++ : F++;

                QJsonObject  face;
                face.insert("x",(float(bboxes.at(it).x - padding ) / float(ncMat.w)));
                face.insert("y",(float(bboxes.at(it).y - padding) / float(ncMat.h)));
                face.insert("w",(float(bboxes.at(it).width + padding) / float(ncMat.w)));
                face.insert("h",(float(bboxes.at(it).height + padding) / float(ncMat.h)));
                face.insert("gender",gender);
                face.insert("age",age);
                faces.append(face);

            }  // for loop
            // }); // parallel_for_

            detection.insert("male_count",M);
            detection.insert("female_count",F);
            detection.insert("viewers",faces);

            emit objectsDetected(QString::fromStdString(QJsonDocument(faces).toJson().toStdString()));

        } // detected bboxes.size()>0

        setCounter(bboxes.size());
    }
    else
    {
        qDebug()<< "not ready yet ... " ;

    }
    isProcessing= false;
}

#ifdef Q_OS_ANDROID

void CVcustomDetector::detect_OCV_RKNN(cv::Mat mat)
{

    isProcessing= true;



    if (!yuNet.empty() && !mat.empty()){


        // QVector<cv::Rect> bboxes = getFaceBox_classifier(&mat);
        // QVector<cv::Rect> bboxes = getFaceBox_dnn(&mat);
        QVector<cv::Rect> bboxes = getFaceBox_yunet(&mat);


        QJsonObject detection;
        detection.insert("total_count",bboxes.size());

        int M=0,F=0;


        if(bboxes.size()==0){
            qDebug()<< "no face detected  " << bboxes.size();
            emit noObject();
        }
        else{ // detected bboxes.size()>0
            qDebug()<< "number of faces detected:  " << bboxes.size();


            QJsonArray faces;


            // cv::parallel_for_(cv::Range(0,bboxes.size()), [&](const cv::Range& range){ // parallel_for_
            // for (auto it = range.start; it < range.end; it++) { //for loop parallel
            for (auto it = 0 /*range.start*/; it < bboxes.size() /*range.end*/; it++) { //for loop


                Rect rec = bboxes.at(it);


                Mat faceRect = mat(rec); // take the ROI of box on the frame
                cv::resize(faceRect,faceRect,Size(224, 224));

                std::vector<float> genderPredsrk;
                rk_gender.inference((uint8_t *) faceRect.data,&genderPredsrk);
                int max_index_genderrk = std::distance(genderPredsrk.begin(), max_element(genderPredsrk.begin(), genderPredsrk.end()));
                QString gender ;
                if (genderPredsrk[max_index_genderrk]> 0.8)
                    gender = genderList[max_index_genderrk];
                else
                    continue;


                std::vector<float> agePredsrk;
                rk_age.inference((uint8_t *) faceRect.data,&agePredsrk);
                int max_indice_agerk = std::distance(agePredsrk.begin(), max_element(agePredsrk.begin(), agePredsrk.end()));
                QString age = ageList[max_indice_agerk];



                qDebug()<<  "RKNN gender/conf: "<<  gender <<"/"<< genderPredsrk[max_index_genderrk] <<" age/conf: "<< age<<"/"<<agePredsrk[max_indice_agerk] ;

                QJsonObject  face;
                face.insert("x",(float(rec.x) / float(mat.cols)));
                face.insert("y",(float(rec.y) / float(mat.rows)));
                face.insert("w",(float(rec.width) / float(mat.cols)));
                face.insert("h",(float(rec.height) / float(mat.rows)));
                face.insert("gender",gender);
                face.insert("age",age);
                faces.append(face);

            }  // for loop
            // }); // parallel_for_

            detection.insert("male_count",M);
            detection.insert("female_count",F);
            detection.insert("viewers",faces);

            emit objectsDetected(QString::fromStdString(QJsonDocument(faces).toJson().toStdString()));

        } // detected bboxes.size()>0

        setCounter(bboxes.size());
    }
    else
    {
        qDebug()<< "not ready yet ... " ;

    }

    isProcessing= false;

}

#endif
