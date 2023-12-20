# libffmpeg-zmq-streaming

Originally, this code would encode camera or directory images to h264 and just send
those packets via zmq to a subscriber.

I have since enabled RTP transport, so the stream can be played with ffplay and VLC
(with h264 only).

## Building
If you have autocrane-deps installed you most likely have all dependencies already.
To use the deps from autocrane folder run cmake like:
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="PATH/TO/autocrane-core/deps/"
```

## Binaries

`encode_video_fromdir` reads images from a directory and sends it to an rtp host and port. An SDP file is written and
needs to be used by the client (does not change unless you change parameters).

Usage is

```
./build/encode_video_fromdir ~/Downloads/images/ jpeg 127.0.0.1 5006
```

Beware that **an even-numbered RTP port** is necessary otherwise VLC will not receive
packets. This is because the live555 library VLC used discards the last bit of the port
number, so the port gets changed when odd (wtf).

`encode_from_zmq` receives images via zmq and sends it to an rtp host and port. Check [below](#Encode-From-ZMQ) for details.

# Streaming to VLC

The current code uses VP9, which won't work with VLC, but the h264 stuff is still there
commented out.

VLC can stream this with

```
/Applications/VLC.app/Contents/MacOS/VLC -vvv test.sdp
```

On ios, you can also use the VLC app and then follow these steps.

1. Serve the SDP file over HTTP somehow – there is no way to open a file on the device
   itself. For test purposes you can run `python3 -m http.server` from the directory
   where the SDP file is located
2. Open the VLC app, go to **Network**, then enter `http://<host ip>:8000/test.sdp` and
   tap **Open Network Stream**
3. Start the stream on the host with `./build/encode_video_zmq ~/Downloads/images/ jpeg <ios ip> 5006` or similar
4. The stream should now appear

This streaming process has a delay of at least 0.5s, which I could not get down, even with
`--network-caching=0`

## Streaming to `ffplay`

The lowest-latency invocation I have found is

```
ffplay -probesize 32 -analyzeduration 0 -fflags nobuffer -fflags discardcorrupt -flags low_delay -sync ext -framedrop -avioflags direct -protocol_whitelist "file,udp,rtp" test.sdp
```

And that also has 200ms delay.

You can use `decode_rtp <sdpfile>` binary to be a little better. You can change the
`max_delay` to adjust reorder tolerance in the code, or disable reordering by setting it to 0. This doesn't work over lossy links though.

# Dependencies

Unfortunately this is a bit shitty because there is no cmake support for libffmpeg. I pilfered a cmake script for finding ffmpeg from VTK (i think),
but they do not include a bunch of library dependencies (not sure if forgotten or not necessary for certain versions of libffmpeg), so I hacked them in there until it worked.

Basically you need most (but strangely not all) thats outlined here: https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu

Depending on which codecs you have enabled during ffmpeg install
(`--enable-libtheora`, `--enable-libvpx`, `--enable-libx264`), you need to adapt the
`FindFFMPEG.cmake` script and add to `libdeps`.

- Theora: `theora;theoraenc;vorbis;vorbisenc`
- x264: `x264`
- vpx: `vpx`

Theora I actually could not get to work because libvacodec complains that `Configuration
is missing` or something. Not sure if I accidentally fixed this by now.

VP9 packetization for RTP transport is experimental, so you need

```
this->ofmt_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
```

However, VLC does not seem to support that so the point is moot, unless you can use
ffplay or write your own receiver with libavcodec.

This code needs ffmpeg version 4.4 right now, because it would need to update for their
breaking API changes. If you have that installed to some prefix that's not a system
prefix, use `-DFFMPEG_ROOT=<prefix>` during `cmake`.

# Performance

When sender and receiver run on the same host, no streaming delay is observed, save for
the time it takes to encode and decode. There is not a single frame of delay, so the
method can be considered to be optimal on a lossless link.

Over a VPN connection via Azure to the same country (germany), performance is still very
good, the loss is low enough that only rarely a frame is missed.

Over a VPN connection via Azure US and a much more delayed and lossly link, this is
unuseable, as over UDP seemingly too few packets get lost to even decode a single frame
in time. We would need to try TCP for this, which `libavcodec` does not support for RTP.
RTSP would have to be investigated.

Previously, I was sending plain h264 packets over Zmq, which can use TCP. Performance
for this was tolerable for high resolution images, with some artifacts and jankiness,
but might be improved with smaller image size. Special consideration must be taken with
setting `conflate`, `sendhwm` and `recvhwm` options in senders/receivers to avoid
unintentional buffering and delays.

# Encode From ZMQ

To build this binary, you  must pass `-DBUILD_zmq_encoder=ON` to cmake, and have zeromq,
cppzmq, Poco and LibDataChannel installed. Refer to other repos (`psiori-streaming`) for
how to install them.

This tool is intended to receive images from somewhere via ZMQ and connect to
`psiori-streaming` mediaserver to publish this data as a video stream.

Example usage is

```
./encode_from_zmq --zmq-port 6004 --zmq-host 127.0.0.1 --zmq-user operator --zmq-password psiori --topic testsrc
```

```

USAGE:

   ./third_party/libffmpeg-zmq-streaming/bin/encode_from_zmq  [--bgr] [-b
                                        <Bitrate as Integer>] [-f <fps as
                                        Integer>] [--stream-port <Port>]
                                        [-m <Ip address>] -t <topic as
                                        string> --mediaserver-password
                                        <password as string>
                                        [--zmq-password <password as
                                        string>] -u <user as string>
                                        --zmq-port <Port as Integer> [-H
                                        <Host as String>] [--] [--version]
                                        [-h]


Where:

   --bgr
     switch channels before sending

   -b <Bitrate as Integer>,  --bitrate <Bitrate as Integer>
     bitrate of stream

   -f <fps as Integer>,  --fps <fps as Integer>
     fps of stream

   --stream-port <Port>
     WebSocket port on the mediaserver for setup

   -m <Ip address>,  --mediaserver <Ip address>
     mediaserver IP

   -t <topic as string>,  --topic <topic as string>
     (required)  topic of the stream

   --mediaserver-password <password as string>
     (required)  password for mediaserver authentication

   --zmq-password <password as string>
     password for zmq authentication

   -u <user as string>,  --zmq-user <user as string>
     (required)  user for zmq authentication

   --zmq-port <Port as Integer>
     (required)  port of incoming zmq messages

   -H <Host as String>,  --zmq-host <Host as String>
     host of incoming zmq messages

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.


   Recieve zmq (from KAI-core), send via RTP

```
