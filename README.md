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
