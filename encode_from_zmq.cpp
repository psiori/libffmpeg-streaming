#include <atomic>
#include <boost/thread/sync_bounded_queue.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <tclap/CmdLine.h>
#include <thread>
#include <zmq.hpp>

#include "avtransmitter.hpp"
#include "time_functions.hpp"

using std::string;
using namespace std::chrono;
using namespace TCLAP;

class VideoStreamMonitor {
private:
  AVTransmitter transmitter;
  std::thread encoder;
  std::atomic<bool> stop;
  typedef boost::sync_bounded_queue<cv::Mat> Queue;
  Queue queue;


  bool has_printed_sdp;

public:
  VideoStreamMonitor(const string& host,
                     unsigned int port,
                     unsigned int fps,
                     unsigned int bitrate);

  ~VideoStreamMonitor();

  void process(cv::Mat data);
};

VideoStreamMonitor::VideoStreamMonitor(const string& host,
                                       unsigned int port,
                                       unsigned int fps,
                                       unsigned int bitrate) :
    transmitter(host, port, fps, 1, bitrate), queue(1), has_printed_sdp(false) {
  stop.store(false);
  av_log_set_level(AV_LOG_QUIET);
  std::cout << "Streaming to " << host << ":" << port << std::endl;
  encoder = std::thread([&]() {
    while (!stop.load()) {
      // const auto queue_status = queue.try_pull_front(image);
      //  keep the zoom_factor value in range [0.2, 1.0]
      cv::Mat image = queue.pull_front();
      // if (queue_status == boost::concurrent::queue_op_status::empty) {
      //   std::this_thread::sleep_for(std::chrono::milliseconds(5));
      //   continue;
      // }
      std::cout << "Begin encode " << std::setprecision(5) << std::fixed
                << duration_cast<milliseconds>(
                       system_clock::now().time_since_epoch())
                           .count() /
                       1000.0
                << std::endl;
      auto tic = current_millis();
      transmitter.encode_frame(image);
      std::cout << "Took " << 1000 * (current_millis() - tic) << std::endl;
      std::cout << "Encoded at " << std::setprecision(5) << std::fixed
                << duration_cast<milliseconds>(
                       system_clock::now().time_since_epoch())
                           .count() /
                       1000.0
                << std::endl;

      if (!has_printed_sdp) {
        has_printed_sdp = true;
        string sdp      = transmitter.get_sdp();
        size_t index    = 0;
        std::cout << "SDP file is: \r\n" << sdp << std::endl;
        std::cout << "------------\r\nSDP file ended" << std::endl;
      }
      // data->releaseLock();
    }
  });
}

VideoStreamMonitor::~VideoStreamMonitor() {
  std::cout << "Deleting Monitor..." << std::endl;
  stop.store(true);
  encoder.join();
  queue.close();
}

void VideoStreamMonitor::process(cv::Mat data) {
  if (!queue.full()) {
    // data->getLock();
    queue.push_back(data);
  }
}

int main(int argc, char* argv[]) {
  CmdLine cmdline("Recieve zmq (from KAI-core), send via RTP");

  ValueArg<string> zmq_host("H", "host", "host of incoming zmq messages", false,
                            "127.0.0.1", "Host as String", cmdline);

  ValueArg<int> zmq_port("", "port", "port of incoming zmq messages", false,
                         6001, "Port as Integer", cmdline);
  ValueArg<string> zmq_user("u", "user", "user to authenticate at zmq", false,
                            "developer", "user as string", cmdline);
  ValueArg<string> zmq_pw("p", "password", "password to authenticate at zmq",
                          false, "psiori", "password as string", cmdline);
  ValueArg<string> zmq_topic("t", "topic", "topic to filter zmq messages",
                             false, "", "topic as string", cmdline);

  ValueArg<string> rtp_host("R", "receiver", "receiver of the rtp stream",
                            false, "127.0.0.1", "Receiver as String", cmdline);
  ValueArg<int> rtp_port("", "stream-port", "port of stream", false, 8000,
                         "Port as Integer", cmdline);
  ValueArg<int> rtp_fps(
      "f", "fps", "fps of stream", false, 20, "fps as Integer", cmdline);
  ValueArg<long> rtp_bitrate("b", "bitrate", "bitrate of stream", false, 100000,
                             "Bitrate as Integer", cmdline);
  SwitchArg bgr("", "bgr", "switch channels before sending", cmdline, true);
  cmdline.parse(argc, argv);
  zmq::context_t ctx(1);
  zmq::socket_t socket(ctx, ZMQ_SUB);

  const string host  = zmq_host.getValue();
  int port           = zmq_port.getValue();
  const string user  = zmq_user.getValue();
  const string pw    = zmq_pw.getValue();
  const string topic = zmq_topic.getValue();

  socket.set(zmq::sockopt::plain_username, user);
  socket.set(zmq::sockopt::plain_password, pw);
  socket.set(zmq::sockopt::rcvhwm, 2);
  socket.set(zmq::sockopt::subscribe, topic);
  const auto connect_str = string("tcp://") + host + ":" + std::to_string(port);
  socket.connect(connect_str);
  std::cout << "Connected zmq socket to " << connect_str
            << ", on topic: " << topic << std::endl;
  std::cout << "Starting main" << std::endl;

  VideoStreamMonitor streamer(rtp_host.getValue(), rtp_port.getValue(),
                              rtp_fps.getValue(), rtp_bitrate.getValue());

  zmq::message_t recv_topic;
  zmq::message_t request;
  zmq::recv_result_t result;
  auto cycle_tic = current_millis();
  while (true) {
    result = socket.recv(recv_topic, zmq::recv_flags::none);
    std::cout << "Got topic, waiting for zmq result" << std::endl;
    result                = socket.recv(request, zmq::recv_flags::none);
    const string topicStr = recv_topic.to_string();
    std::cout << "Received zmq message on topic " << topicStr << std::endl;
    if (topicStr.find("camera|") != string::npos) {
      int64_t more = socket.get(zmq::sockopt::rcvmore);
      uchar* ptr   = reinterpret_cast<uchar*>(request.data());
      std::vector<uchar> data(ptr, ptr + request.size());
      auto tic      = current_millis();
      cv::Mat image = cv::imdecode(data, -1);
      std::cout << "cv::imdecode Took " << 1000 * (current_millis() - tic)
                << std::endl;
      if (bgr.getValue()) {
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
      }
      streamer.process(image);
      if (more == 1) {
        // ROI data, but we dont need them, but they are still received here so
        // they dont show up in the next cycle...
        socket.recv(request, zmq::recv_flags::none);
      }
    } else {
      std::cout << "Skipped non-camera message" << std::endl;
    }
    std::cout << "Cycle-Frequency: " << 1 / (current_millis() - cycle_tic)
              << " Hz" << std::endl;
    cycle_tic = current_millis();
  }
}
