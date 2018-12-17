#include "visionworks.h"

VisionWorks::VisionWorks()
{
}



QImage VisionWorks::MatToQImage(const cv::Mat &inputMat)
{
    switch (inputMat.type())
    {
        // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image(inputMat.data,
            inputMat.cols, inputMat.rows,
            static_cast<int>(inputMat.step),
            QImage::Format_ARGB32);

        return image;
    }

    // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image(inputMat.data,
            inputMat.cols, inputMat.rows,
            static_cast<int>(inputMat.step),
            QImage::Format_RGB888);

        return image.rgbSwapped();
    }

    // 8-bit, 1 channel
    case CV_8UC1:
    {
#if QT_VERSION >= 0x050500

        // From Qt 5.5
        QImage image(inMat.data, inMat.cols, inMat.rows,
                     static_cast<int>(inMat.step),
                     QImage::Format_Grayscale8);
#else
        static QVector<QRgb>  sColorTable;

        // only create our color table the first time
        if (sColorTable.isEmpty())
        {
            sColorTable.resize(256);
            for (int i = 0; i < 256; ++i)
            {
                sColorTable[i] = qRgb(i, i, i);
            }
        }

        QImage image(inputMat.data,
            inputMat.cols, inputMat.rows,
            static_cast<int>(inputMat.step),
            QImage::Format_Indexed8);

        image.setColorTable(sColorTable);
#endif
    }

    default:
        std::cout << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inputMat.type();
        break;
    }

    return QImage();
}


QImage VisionWorks::Mat3bToQImage (cv::Mat3b src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    for (int y = 0; y < src.rows; ++y) {
        const cv::Vec3b *srcrow = src[y];
        QRgb *destrow = (QRgb*)dest.scanLine(y);
        for (int x = 0; x < src.cols; ++x) {
            destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
        }
    }
    return dest;
}


void VisionWorks::VXToMatC1(vx_context m_vxCtx, vx_image& vxiSrc, cv::Mat& matDst)
{
    vx_imagepatch_addressing_t dst_addr;
    dst_addr.dim_x = matDst.cols;
    dst_addr.dim_y = matDst.rows;
    dst_addr.stride_x = sizeof(vx_uint8);
    dst_addr.stride_y = matDst.step;
    void* dst_ptrs[] = {matDst.data};
    vx_image vxiDst = vxCreateImageFromHandle(m_vxCtx, VX_DF_IMAGE_U8, &dst_addr, dst_ptrs, VX_IMPORT_TYPE_HOST);

    nvxuCopyImage(m_vxCtx, vxiSrc, vxiDst);

    vxReleaseImage(&vxiDst);
}

void VisionWorks::MatToVXC1 (vx_context m_vxCtx, cv::Mat& matSrc, vx_image& vxiDst) {
    vx_imagepatch_addressing_t src_addr;
    src_addr.dim_x = matSrc.cols;
    src_addr.dim_y = matSrc.rows;
    src_addr.stride_x = sizeof(vx_uint8);
    src_addr.stride_y = matSrc.step;
    void* src_ptrs[] = {matSrc.data};
    vxiDst = vxCreateImageFromHandle(m_vxCtx, VX_DF_IMAGE_U8, &src_addr, src_ptrs, VX_IMPORT_TYPE_HOST);
}

void VisionWorks::VXToMat(vx_context m_vxCtx, vx_image& vxiSrc, cv::Mat& matDst) {
    vx_image vxr = vxCreateImage(m_vxCtx, matDst.cols, matDst.rows, VX_DF_IMAGE_U8);
    vx_image vxg = vxCreateImage(m_vxCtx, matDst.cols, matDst.rows, VX_DF_IMAGE_U8);
    vx_image vxb = vxCreateImage(m_vxCtx, matDst.cols, matDst.rows, VX_DF_IMAGE_U8);

    cv::Mat cvr = cv::Mat(matDst.rows, matDst.cols, CV_8UC1);
    cv::Mat cvg = cv::Mat(matDst.rows, matDst.cols, CV_8UC1);;
    cv::Mat cvb = cv::Mat(matDst.rows, matDst.cols, CV_8UC1);;

    NVXIO_SAFE_CALL(vxuChannelExtract(m_vxCtx, vxiSrc, VX_CHANNEL_R, vxr));
    NVXIO_SAFE_CALL(vxuChannelExtract(m_vxCtx, vxiSrc, VX_CHANNEL_G, vxg));
    NVXIO_SAFE_CALL(vxuChannelExtract(m_vxCtx, vxiSrc, VX_CHANNEL_B, vxb));
    VXToMatC1(m_vxCtx, vxr, cvr);
    VXToMatC1(m_vxCtx, vxg, cvg);
    VXToMatC1(m_vxCtx, vxb, cvb);

    cv::vector<cv::Mat> channels;
    channels.push_back(cvb);
    channels.push_back(cvg);
    channels.push_back(cvr);

    cv::merge(channels, matDst);
    NVXIO_SAFE_CALL(vxReleaseImage(&vxr));
    NVXIO_SAFE_CALL(vxReleaseImage(&vxg));
    NVXIO_SAFE_CALL(vxReleaseImage(&vxb));

    cvr.release();
    cvg.release();
    cvb.release();
    channels.clear();
}

void VisionWorks::MatToVX (vx_context m_vxCtx, cv::Mat& matSrc, vx_image& vxiSrc) {
    cv::Mat bgr[3];
    cv::split(matSrc,bgr);
    vx_image vxr, vxg, vxb;

    MatToVXC1(m_vxCtx, bgr[0], vxb);
    MatToVXC1(m_vxCtx, bgr[1], vxg);
    MatToVXC1(m_vxCtx, bgr[2], vxr);

   // vxiSrc = vxCreateImage(m_vxCtx, matSrc.cols, matSrc.rows, VX_DF_IMAGE_RGB);
    NVXIO_SAFE_CALL(vxuChannelCombine(m_vxCtx, vxb, vxg, vxr, NULL, vxiSrc));

    NVXIO_SAFE_CALL( vxReleaseImage(&vxr));
    NVXIO_SAFE_CALL( vxReleaseImage(&vxg));
    NVXIO_SAFE_CALL( vxReleaseImage(&vxb));

    bgr[0].release();
    bgr[1].release();
    bgr[2].release();
    delete [] bgr[0].data;
    delete [] bgr[1].data;
    delete [] bgr[2].data;

}
