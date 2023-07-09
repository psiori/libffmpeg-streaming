#pragma once

#include <QImage>
#include <opencv2/opencv.hpp>

namespace qt_client_utils {
	/**
	 * @brief Convert an OpenCV image to QImage.
	 * @param cvImg input cv::Mat
	 * @param display_width
	 * @param cvdisplay_height
	 * @return converted QImage
	 * */
	QImage cvMatToQImage(const cv::Mat& cvImg,
											unsigned int display_width,
											unsigned int display_height);
}  // namespace qt_client_utils
