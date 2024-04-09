# Scripts
This folder contains scripts to automate testing.

**`experiment.sh`**
Automatically runs encoding with different parameters for all videos in `./recordings`. Results are stored in `./results`.
Only tested on macOS; needs `coreutils` (`brew install coreutils`)

**extract_frame.sh**
Extracts specific frames from videos in `./results` (encoded videos) and `./recordings` (raw videos) and stores them to `./frames`. Also extracts frames from videos containing ROI-maps, which must be stored in `./roi-maps`. Usage: `./extract_frame.sh <video> <frame_num>`