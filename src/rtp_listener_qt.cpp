#include <iostream>
#include <chrono>

#include "rtp_listener_qt.hpp"
#include "avutils.hpp"
#include "time_functions.hpp"

RTPListenereQt::RTPListenereQt(const std::string& sdp_path, QObject* parent) : 
QObject{parent} {
	stop.store(false);
	pause.store(false);

	av_log_set_level(AV_LOG_TRACE);
	fmt_ctx = avformat_alloc_context();
	fmt_ctx->flags |= (AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_DISCARD_CORRUPT |
											AVFMT_FLAG_FLUSH_PACKETS);
	av_opt_set(fmt_ctx, "protocol_whitelist", "file,rtp,udp", 0);
	av_opt_set_int(fmt_ctx, "fpsprobesize", 0, 0);
	av_opt_set_int(fmt_ctx, "probesize", 32, 0);
	av_opt_set_int(fmt_ctx, "analyzeduration", 0, 0);
	// do set to 0 over lossy network, fucks it up and you get
	// Invalid data in avcodec_send_packet()
	// we accept 0.1s reordering delay
	fmt_ctx->max_delay = 1'000'000 / 10;

	fmt_ctx->interrupt_callback.opaque   = static_cast<void*>(this);
	fmt_ctx->interrupt_callback.callback = &RTPListenereQt::should_interrupt;

	/* open input file, and allocate format context */
	if (avformat_open_input(&fmt_ctx, sdp_path.c_str(), NULL, NULL) < 0) {
		throw std::invalid_argument("Could not open SDP path " + sdp_path);
	}

	current_packet = new AVPacket;
	codec          = avcodec_find_decoder(AV_CODEC_ID_VP9);
	if (!codec) {
		throw std::invalid_argument("Could not find decoder");
	}

	dec_ctx = avcodec_alloc_context3(codec);

	dec_ctx->thread_count = 1;
	dec_ctx->codec_id     = AV_CODEC_ID_VP9;
	dec_ctx->codec_type   = AVMEDIA_TYPE_VIDEO;
	dec_ctx->pix_fmt      = AV_PIX_FMT_YUV420P;
	dec_ctx->delay        = 0;
	/* dec_ctx->thread_type = FF_THREAD_SLICE; */
	std::cout << std::setprecision(5) << std::fixed << std::endl;

	if (avcodec_open2(dec_ctx, codec, nullptr) < 0) {
		throw std::invalid_argument("Could not open context");
	}
	current_frame = av_frame_alloc();

	runner = std::thread([&]() {
		while (!stop.load()) {
			while (!pause.load() && av_read_frame(fmt_ctx, current_packet) >= 0) {
				auto packet_received = system_clock::now();
				int success          = avcodec_send_packet(dec_ctx, current_packet);
				auto packet_sent     = system_clock::now();
				av_packet_unref(current_packet);
				if (success != 0) {
					std::cout << "Could not send packet: "
										<< avutils::av_strerror2(success) << std::endl;
					continue;
				}
				success             = avcodec_receive_frame(dec_ctx, current_frame);
				auto frame_received = system_clock::now();
				if (success == 0) {
					if (!sws_ctx) {
						sws_ctx =
								sws_getContext(current_frame->width, current_frame->height,
																AV_PIX_FMT_YUV420P, current_frame->width,
																current_frame->height, dst_fmt_, SWS_BILINEAR,
																NULL, NULL, NULL);
					}

					AVFrame* rgb_frame     = av_frame_alloc();
					rgb_frame->width       = current_frame->width;
					rgb_frame->height      = current_frame->height;
					rgb_frame->format      = dst_fmt_;
					rgb_frame->linesize[0] = rgb_frame->width * 4;
					/* rgb_frame->data[0] = */
					/*     new uint8_t[rgb_frame->width * rgb_frame->height * 4 + 16];
						*/
					av_image_alloc(rgb_frame->data, rgb_frame->linesize,
													rgb_frame->width, rgb_frame->height, dst_fmt_, 16);

					int slice_h = sws_scale(
							sws_ctx, current_frame->data, current_frame->linesize, 0,
							rgb_frame->height, rgb_frame->data, rgb_frame->linesize);
					std::vector<int> sizes{rgb_frame->height, rgb_frame->width};
					std::vector<size_t> steps{
							static_cast<size_t>(rgb_frame->linesize[0])};
					cv::Mat image(sizes, CV_8UC4, rgb_frame->data[0], &steps[0]);
					auto image_created = system_clock::now();
					if (!image.empty()) {
						emit imageReceived(image);
					}
					/* stamp_image(image, packet_received, 0.2); */
					/* stamp_image(image, packet_sent, 0.4); */
					/* stamp_image(image, image_created, 0.6); */
					av_freep(&rgb_frame->data[0]);
					std::cout << "Packet received: "
										<< format_timepoint_iso8601(packet_received) << std::endl;
					std::cout << "Packet sent: "
										<< format_timepoint_iso8601(packet_sent) << std::endl;
					std::cout << "Frame received: "
										<< format_timepoint_iso8601(frame_received) << std::endl;
					std::cout << "Image stamped: "
										<< format_timepoint_iso8601(system_clock::now())
										<< std::endl;
					av_frame_free(&rgb_frame);
					av_frame_unref(current_frame);
				} else {
					std::cout << "Did not get frame " << avutils::av_strerror2(success)
										<< std::endl;
				}
			}
			std::cout << "Exited recv loop" << std::endl;
		}
	});
}

RTPListenereQt::~RTPListenereQt() {
	pause.store(true);
	stop.store(true);
	runner.join();
	avformat_close_input(&fmt_ctx);
	avcodec_free_context(&dec_ctx);
	av_frame_free(&current_frame);
	av_packet_unref(current_packet);
	delete current_packet;
}