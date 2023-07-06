#pragma once

#include <QImage>
#include <QObject>
#include <QThread>
#include <opencv2/opencv.hpp>

#include "rtp_receiver.hpp"

class VideoThread : public QThread {
  Q_OBJECT

public:
  /**
   * @brief Construct a new VideoThread instance.
   * @param image_iterator Image Iterator
   * */
  explicit VideoThread(RTPReceiver& rtp_receiver,
                       QObject* parent = nullptr);
  void run() override;
  void stop();
  ~VideoThread();

signals:
  /**
   * @brief Signal that will be emitted when image is loaded from disk. Signal
   * is connected to slot to update displayed image.
   * @param const QImage loaded from disk
   */
  void changePixmap(const QImage& image);

private:
  /** var to start and stop thread */
  bool _runFlag = false;
  
  RTPReceiver& rtp_receiver_;
};