QT += quick multimedia websockets


HEADERS += \
    cvcustomdetector.h

SOURCES += \
        cvcustomdetector.cpp \
        main.cpp

resources.files = main.qml SettingsScreen.qml
resources.prefix = /$${TARGET}
RESOURCES += resources \
    media.qrc \
    ncnnModels.qrc \
    faceModel.qrc

CONFIG += resources_big

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



unix:!android: {
  INCLUDEPATH += /usr/include/
  INCLUDEPATH += /usr/include/opencv4/

  LIBS += -lopencv_core \
          -lopencv_imgproc \
          -lopencv_dnn \
          -lopencv_photo \
          -lopencv_video \
          -lopencv_highgui \
          -lopencv_imgcodecs \
          -lopencv_videoio \
          -lopencv_flann \
          -lopencv_features2d \
          -lopencv_calib3d \
          -lopencv_objdetect \
          -lopencv_stitching \
          -lopencv_ml

    INCLUDEPATH += /usr/include/ncnn/
    LIBS += -lncnn
}


android {
    HEADERS += \
        rknn.h

    SOURCES += \
            rknn.cpp
    RESOURCES += rknnModels.qrc

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    OTHER_FILES +=  \
                    android/AndroidManifest.xml \
                    android/build.gradle \
                    android/gradle.properties \
                    android/gradle/wrapper/gradle-wrapper.jar \
                    android/gradle/wrapper/gradle-wrapper.properties


    ANDROID_MIN_SDK_VERSION = 26
    ANDROID_TARGET_SDK_VERSION = 34


    contains(ANDROID_TARGET_ARCH,arm64-v8a) {

        ANDROID_ABIS="arm64-v8a"
        ANDROID_PACKAGE_SOURCE_DIR = \
            $$PWD/android




        ## opencv
        INCLUDEPATH += $$PWD/3rdparty/opencv/4.10/sdk/native/jni/include/

        LIBS += -L$$PWD/3rdparty/opencv/4.10/sdk/native/staticlibs/arm64-v8a/
        LIBS += \
                -lopencv_calib3d \
                -lopencv_core \
                -lopencv_dnn \
                -lopencv_features2d \
                -lopencv_flann \
                -lopencv_gapi \
                -lopencv_highgui \
                -lopencv_imgcodecs \
                -lopencv_imgproc \
                -lopencv_ml \
                -lopencv_objdetect \
                -lopencv_photo \
                -lopencv_stitching \
                -lopencv_video \
                -lopencv_videoio \


        LIBS += -L$$PWD/3rdparty/opencv/4.10/sdk/native/3rdparty/libs/arm64-v8a/
        LIBS += \
                 -lade \
                 -lcpufeatures \
                 -lIlmImf \
                 -littnotify \
                 -lkleidicv \
                 -lkleidicv_hal \
                 -llibjpeg-turbo \
                 -llibopenjp2 \
                 -llibpng \
                 -llibprotobuf \
                 -llibtiff \
                 -llibwebp \
                 -ltbb \
                 -ltegra_hal \
                 -lzlib \


        ## ncnn
        INCLUDEPATH += $$PWD/3rdparty/ncnn/android/arm64-v8a/include/
        LIBS += -L$$PWD/3rdparty/ncnn/android/arm64-v8a/lib/staticlibs/
        LIBS += -lncnn \
                -lMachineIndependent  \
                -lOSDependent  \
                -lOGLCompiler  \
                -lglslang \
                -lglslang-default-resource-limits  \
                -lGenericCodeGen  \
                -lSPIRV \
                -landroid \ # requeired by libncnn
                -lomp \     # requeired by libncnn  // copy from NDK toolchain (llvm) directory (toolchains/llvm/prebuilt/linux-x86_64/lib/clang/17/lib/linux/aarch64/)

        # LIBS += -L/opt/android/SDK/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/17/lib/linux/aarch64/ -lomp  # this will select dyynamic linked libomp.so

        ## RKNN
        INCLUDEPATH += $$PWD/3rdparty/rknn/librknn_api/include/
        LIBS += -L$$PWD/3rdparty/rknn/librknn_api/arm64-v8a/
        LIBS += -lrknnrt
        ANDROID_EXTRA_LIBS += \
                $$PWD/3rdparty/rknn/librknn_api/arm64-v8a/librknnrt.so

    }


    contains(ANDROID_TARGET_ARCH,armeabi-v7a) {

        ANDROID_ABIS="armeabi-v7a"
        ANDROID_PACKAGE_SOURCE_DIR = \
            $$PWD/android

        ## opencv
        INCLUDEPATH += $$PWD/3rdparty/opencv/4.10/sdk/native/jni/include/
        LIBS += -L$$PWD/3rdparty/opencv/4.10/sdk/native/staticlibs/armeabi-v7a/
        LIBS += -pthread -llibprotobuf -lpthread \
                -lopencv_core \
                -lopencv_imgproc \
                -lopencv_dnn \
                -lopencv_photo \
                -lopencv_video \
                -lopencv_highgui \
                -lopencv_imgcodecs \
                -lopencv_videoio \
                -lopencv_flann \
                -lopencv_features2d \
                -lopencv_calib3d \
                -lopencv_objdetect \
                -lopencv_stitching \
                -lopencv_ml

        LIBS += -L$$PWD/3rdparty/opencv/4.10/sdk/native/3rdparty/libs/armeabi-v7a/
        LIBS += -lade \
                -lcpufeatures \
                -lIlmImf \
                -littnotify \
                -llibjpeg-turbo \
                -llibopenjp2 \
                -llibpng \
                -llibprotobuf \
                -llibtiff \
                -llibwebp \
                -ltbb \
                -ltegra_hal


        ## ncnn
        QMAKE_CXXFLAGS += -static-openmp
        QMAKE_CFLAGS   += -static-openmp
        INCLUDEPATH += $$PWD/3rdparty/ncnn/android/armeabi-v7a/include/
        LIBS += -L$$PWD/3rdparty/ncnn/android/armeabi-v7a/lib/
        LIBS += -lncnn -lomp -landroid \
                -lOGLCompiler  \
                -lOSDependent  \
                -lGenericCodeGen  \
                -lglslang-default-resource-limits  \
                -lMachineIndependent  \
                -lglslang \
                -lSPIRV

    }
}
