This is a modified version of x264 which includes an custom AQ (adaptive quantization) mode. The original source code of x264 can be found [here](https://code.videolan.org/videolan/x264).

# Compilation
1. Install `czmq` (e.g. via `brew install czmq`)
2. `./configure --extra-ldflags="-lczmq -lzmq`
   If `czmq` is not found specfify paths:
   e.g. `./configure --enable-debug --extra-cflags="-I/opt/homebrew/include" --extra-ldflags="-L/opt/homebrew/lib -lczmq -lzmq"` 
3. `make`

# Usage
`./x264 -o [outfile] --aq-mode 4 [infile]`
AQ-mode 4 is the new custom mode.