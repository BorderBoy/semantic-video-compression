This is a modified version of x264 which includes an custom AQ (adaptive quantization) mode. The original source code of x264 can be found [here](https://code.videolan.org/videolan/x264).

# Compilation
1. `./configure`
2. `make`

# Usage
`./x264 -o [outfile] --aq-mode 4 [infile]`
AQ-mode 4 is the new custom mode.