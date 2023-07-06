#include "video_thread.hpp"

#include <QPixmap>
#include <chrono>
#include <iostream>
#include <thread>

VideoThread::VideoThread(RTPReceiver& rtp_receiver, QObject* parent) :
    QThread(parent), _runFlag(true), rtp_receiver_(rtp_receiver) {
}

void VideoThread::run() {
  while (_runFlag) {
    const cv::Mat cvImg = rtp_receiver_.get();
    if (!cvImg.empty()) {
			QImage qImg(cvImg.data, cvImg.cols, cvImg.rows,
                static_cast<int>(cvImg.step), QImage::Format_RGB888);
    	emit changePixmap(qImg);
		}
  }
}

void VideoThread::stop() {
  _runFlag = false;
  wait();
}

VideoThread::~VideoThread() {
  stop();
}
