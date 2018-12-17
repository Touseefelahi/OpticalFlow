#-------------------------------------------------
#
# Project created by QtCreator 2018-12-13T12:37:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VisionWorksGraph
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    visionworks.cpp

HEADERS  += mainwindow.h \
    visionworks.h

FORMS    += mainwindow.ui


CONFIG += c++11

#Opencv Linking
Opencv_Headers = "/usr/include/opencv/"  # Path to opencv headers
Opencv2_Headers = "/usr/include/opencv2/"  # Path to opencv headers

Opencv_LibsDirectory = "/usr/lib/"           # Path to cuda toolkit install
INCLUDEPATH+= $$Opencv_Headers
INCLUDEPATH+= $$Opencv2_Headers

QMAKE_LIBDIR += $$Opencv_LibsDirectory
LIBS+= -lopencv_core -lopencv_flann -lopencv_gpu -lopencv_calib3d -lopencv_contrib -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml  -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_video -lopencv_videostab
#opencvEND


#Including VisionWorks
{
LIBS+=/usr/share/visionworks/sources/3rdparty/glfw3/libs/libglfw3.a
LIBS+=-lXrandr -lXi -lXxf86vm -lX11 -lGL -lfreetype

NvxHeader = "/usr/include/NVX/"  # Path to VisionWorks headers
NvxLibs = "/usr/lib/"           # Path to VisionWorks toolkit install
INCLUDEPATH+= $$NvxHeaders

VisionWorksHeader="/usr/share/visionworks/sources/nvxio/include"
INCLUDEPATH+=$$VisionWorksHeader


QMAKE_LIBDIR += $$NvxLibs

QMAKE_LIBDIR += /usr/lib/arm-linux-gnueabihf
QMAKE_LIBDIR += "/usr/share/visionworks/sources/libs/armv7l/linux/release"
QMAKE_LIBDIR += "/usr/share/visionworks/sources/3rdparty/eigen"
QMAKE_LIBDIR += /usr/share/visionworks/sources/3rdparty/glfw3/libs

LIBS += -lvisionworks -lnvxio -lpthread -lglfw3 -lvisionworks_sfm -lvisionworks_tracking -lXxf86vm -lGL -lgobject-2.0 -lglib-2.0 -lX11 -lfreetype -lXi -lXrandr #-lxrandr -lxi -lxxf86vm
}
#End VisionWorks

#Including Gstreamer
{
Gstreamer =/usr/include/gstreamer-1.0
QMAKE_LIBDIR += /usr/lib/arm-linux-gnueabihf
INCLUDEPATH+=$$Gstreamer
LIBS += -lgstreamer-1.0 -lgstpbutils-1.0 -lgstapp-1.0 -lgstbase-1.0
}
#End Gstreamer



# This makes the .cu files appear in your project
#OTHER_FILES +=  cudaInterface.cu

# CUDA settings <-- may change depending on your system
#CUDA_SOURCES += cudaInterface.cu
CUDA_SDK = "/usr/local/cuda-6.5"  # Path to cuda SDK install
CUDA_DIR = "/usr/local/cuda-6.5"           # Path to cuda toolkit install

# DO NOT EDIT BEYOND THIS UNLESS YOU KNOW WHAT YOU ARE DOING....

SYSTEM_NAME = unix         # Depending on your system either 'Win32', 'x64', or 'Win64'
SYSTEM_TYPE = 32            # '32' or '64', depending on your system
CUDA_ARCH = sm_20           # Type of CUDA architecture, for example 'compute_10', 'compute_11', 'sm_10'
NVCC_OPTIONS = --use_fast_math

CUDA_OBJECTS_DIR = ./


# include paths
INCLUDEPATH += $$CUDA_DIR/include

# library directories
QMAKE_LIBDIR += $$CUDA_DIR/lib/



# Add the necessary libraries
CUDA_LIBS = -lcuda -lcudart -lcublas -lcudnn -lcufft -lcuinj32 -lcurand -lcusparse -lnppc -lnpps -lnvToolsExt -L/usr/local/cuda-6.5/lib

# The following makes sure all path names (which often include spaces) are put between quotation marks
CUDA_INC = $$join(INCLUDEPATH,'" -I"','-I"','"')
LIBS += $$CUDA_LIBS

# Configuration of the Cuda compiler
CONFIG(debug, debug|release) {
    # Debug mode
    cuda_d.input = CUDA_SOURCES
    cuda_d.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda_d.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda_d
}
else {
    # Release mode
    cuda.input = CUDA_SOURCES
    cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda.commands = $$CUDA_DIR/bin/nvcc $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda
}

