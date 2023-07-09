#include "qt_client_utils.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace qt_client_utils {
  QImage cvMatToQImage(const cv::Mat& cvImg,
                      unsigned int display_width,
                      unsigned int display_height) {
    cv::Mat rgbImage;
    cv::cvtColor(cvImg, rgbImage, cv::COLOR_BGR2RGB);
    QImage qImg(rgbImage.data, rgbImage.cols, rgbImage.rows,
                static_cast<int>(rgbImage.step), QImage::Format_RGB888);
    return qImg.scaled(QSize(display_width, display_height), Qt::KeepAspectRatio);
  }
}  // namespace qt_client_utils
