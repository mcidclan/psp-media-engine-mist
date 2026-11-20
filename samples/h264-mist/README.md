# MIST - H264 Simple Sample Code

Sample code demonstrating MIST custom task execution on the Media Engine during H.264 video playback, with sleep/wake and concurrency handling.

## Build

Build and install MIST with the following:
```bash
mkdir -p build && \
cd build && \
cmake -DPRX_FREE=1 .. && \
make clean; make install && \
cd ..
```

Build the sample code from its build folder with:
```bash
cmake .. && make clean; make
```

## Convert your video

convert your video into frames.h264 file using something like:  

```bash
ffmpeg -i video.mp4 -an -c:v libx264 -profile:v main \
  -vf "scale=(iw*272/ih):272,crop=480:272" \
  -r 30 \
  -bsf:v "filter_units=remove_types=6|12|35|38-40" \
  -x264-params "bframes=0:keyint=180:keyint_min=180:open_gop=0:cabac=1:ref=2\
  :8x8dct=0:vbv-maxrate=128Ki:vbv-bufsize=128Ki:vbv-init=1.0:nal-hrd=cbr" \
  -b:v 300k -maxrate 128Ki -bufsize 128Ki -f h264 frames.h264
```

## Execution
Place the EBOOT.PBP and the generated frames.h264 together in one PSP/GAME folder.

*Note: Real hardware only.*  

> [!WARNING]
> Be careful if you're running it through psplink, as some versions have issues with sceIo after sleep.

m-c/d
