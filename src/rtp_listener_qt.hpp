#pragma once

#include <atomic>
#include <thread>
#include <opencv2/highgui.hpp>
#include <QObject>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
}

class RTPListenereQt : public QObject  {
	Q_OBJECT

private:
  AVFormatContext* fmt_ctx;
  AVCodecContext* dec_ctx;
  const AVCodec* codec;
  AVFrame* current_frame;
  AVPacket* current_packet;
  SwsContext* sws_ctx    = nullptr;
  AVPixelFormat dst_fmt_ = AV_PIX_FMT_BGRA;

  std::atomic<bool> stop;
  std::atomic<bool> pause;
  std::thread runner;

  static int should_interrupt(void* opaque) {
    return opaque != nullptr && static_cast<RTPListenereQt*>(opaque)->stop.load();
  }

public:
	RTPListenereQt(const std::string &sdp_path, QObject* parent);

	~RTPListenereQt();

signals:
  /**
   * @brief Signal that will be emitted when image is loaded from disk. Signal
   * is connected to slot to update displayed image.
   * @param const QImage loaded from disk
   */
  void imageReceived(const cv::Mat& cvImage);
};