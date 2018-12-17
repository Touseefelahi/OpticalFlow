#ifndef VISIONWORKS_H
#define VISIONWORKS_H

#include <NVX/nvx.h>
#include <NVX/nvx_timer.hpp>
#include <QImage>
#include <NVXIO/Application.hpp>
#include <NVXIO/ConfigParser.hpp>
#include <NVXIO/FrameSource.hpp>
#include <NVXIO/Render.hpp>
#include <NVXIO/SyncTimer.hpp>
#include <NVXIO/Utility.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <VX/vx.h>
#include <VX/vxu.h>

class VisionWorks
{
public:
    VisionWorks();
    QImage MatToQImage(const cv::Mat& inputMat);
    QImage Mat3bToQImage(cv::Mat3b src);
    void VXToMatC1(vx_context m_vxCtx, vx_image& vxiSrc, cv::Mat& matDst);
    void MatToVXC1 (vx_context m_vxCtx, cv::Mat& matSrc, vx_image& vxiDst);
    void VXToMat(vx_context m_vxCtx, vx_image& vxiSrc, cv::Mat& matDst);
    void MatToVX (vx_context m_vxCtx, cv::Mat& matSrc, vx_image& vxiSrc);
};

#endif // VISIONWORKS_H
