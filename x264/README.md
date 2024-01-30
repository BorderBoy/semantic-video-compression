# x264
This is a modified version of x264 which includes an custom, semantic AQ (adaptive quantization) mode. The original source code of x264 can be found [here](https://code.videolan.org/videolan/x264).

## Compilation
1. Install `czmq` (e.g. via `brew install czmq`)
2. `./configure --extra-ldflags="-lczmq -lzmq`
   If `czmq` is not found specfify paths:
   e.g. `./configure --enable-debug --extra-cflags="-I/opt/homebrew/include" --extra-ldflags="-L/opt/homebrew/lib -lczmq -lzmq"` 
3. `make`

## Usage
**Example:** `./x264 --aq-mode 4 --fps [fps] --input-res [w]x[h] -o [outfile] [infile]`
The parameters `--fps`and `--input-res` are needed when the infile has .YUV format, as those parameters are not stored in the file.
If you want your resulting H.264 stream with WebRTC, make sure to use the baseline profile (`--profile baseline`).
AQ-mode 4 is the new custom mode.