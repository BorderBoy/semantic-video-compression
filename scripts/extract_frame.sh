#!/bin/bash
RECS_PATH=./recordings
ENC_RECS_PATH=./results
ROI_PATH=./roi-maps
RES_PATH=./frames
BITRATES=(750 1500) #in kbit/s
ROI_MODES=("raw" "none" "face" "fes")
FRAME=$2
VIDEO=$1
RESOLUTION="1280x720"

if [[ -z $1 || -z $2 ]]; then
    echo "Usage: ./extract_frame.sh <video> <frame>"
    exit 1
fi

if [ "$VIDEO" == "hololens" ]; then
    RESOLUTION="1128x636"
fi

for mode in "${ROI_MODES[@]}"; do
    if [ "$mode" == "raw" ]; then
        ffmpeg -video_size "$RESOLUTION" -i "$RECS_PATH"/"$VIDEO"_"$RESOLUTION"_30.yuv -vf "select=eq(n\,"$FRAME")" -vframes 1 "$RES_PATH"/"$FRAME"_"$VIDEO"_raw.png
    else
        if [ "$mode" != "none" ]; then
            ffmpeg -i "$ROI_PATH"/"$VIDEO"_"$mode".mp4 -vf "select=eq(n\,"$FRAME")" -vframes 1 "$RES_PATH"/"$FRAME"_"$VIDEO"_"$mode"_roi.png
        fi
        for br in "${BITRATES[@]}"; do
            ffmpeg -i "$ENC_RECS_PATH"/"$VIDEO"_"$RESOLUTION"_30_"$br"_"$mode".h264 -vf "select=eq(n\,"$FRAME")" -vframes 1 "$RES_PATH"/"$FRAME"_"$VIDEO"_"$br"_"$mode".png
        done 
    fi
done


