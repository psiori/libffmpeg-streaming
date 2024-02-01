# libffmpeg-zmq-streaming

Some partly dysfunctional software for video streaming. Includes a small "library" and a
tool to encode from zmq-transmitted rgb frames to a webrtc SFU. See [psiori-streaming] (https://github.com/psiori/psiori-streaming) for the SFU and qt client.

## Building
If you have autocrane-deps installed you most likely have all dependencies already.
To use the deps from autocrane folder run cmake like:

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="PATH/TO/autocrane-core/deps/"
```

# `avutils`

This project includes the `av-streaming` library which you can link into your project.
There is no documentation, refer to this repo's headers to learn what it does.

# Encode From ZMQ

To build this binary, you  must pass `-DBUILD_zmq_encoder=ON` to cmake, and have zeromq,
cppzmq, Poco and LibDataChannel installed. Refer to other repos (`psiori-streaming`) for
how to install them.

This tool is intended to receive images from somewhere via ZMQ and connect to
`psiori-streaming` mediaserver to publish this data as a video stream.

Example usage is

```
./encode_from_zmq --zmq-port 6005 --zmq-host 127.0.0.1 --zmq-user operator --zmq-password psiori --topic cam_trolley_left --stream-port 8080 --mediaserver 46.4.38.241 --mediaserver-password <pw> -b 5000000 --fps 20
```

with the ZMQ params matching autocrane config.

Pass the `--help` argument for more info.

## Sending dummy data without autocrane
When running the `encode_from_zmq` binary, it will print the UDP port it uses
internally, for the stream (zmq images converted to RTP). Since this is just a random
free udp port, you can send any rtp data to it, as long as no autocrane is running to
interfere.

Look for the output

```
WebRTCBridge on topic cam_cabin: Bound internal socket to port 46374
```

and use that port to send data to. You can use ffmpeg to generate test data of any kind,
with the right codec, eg.

```
/home/autocrane/deps/bin/ffmpeg -re -f lavfi -i "testsrc,fps=24" -vcodec libvpx-vp9 -pix_fmt yuv420p -b:v 2M  -g 10 -deadline realtime -cpu-used 8 -strict experimental -f rtp "rtp://127.0.0.1:46374"
```
