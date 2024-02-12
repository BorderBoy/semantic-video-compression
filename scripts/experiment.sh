#!/bin/bash
X264_PATH="../x264/x264"
RECS_PATH=./recordings
RES_PATH=./results
ROI_ANALYZER_PATH=../roi-analyzer/build
BITRATES=(500 1500) #in kbit/s
ROI_MODES=("none" "face" "fes")
FPS=30
RESOLUTION="1280x720"

# clear results file
echo "" > results.csv

for mode in "${ROI_MODES[@]}"; do
    if [ "$mode" != "none" ]; then
        # launch roi analyzer in background
        cd $ROI_ANALYZER_PATH
        ./ROI-Detector -m $mode &
        cd -

        sleep 1
    fi

    for br in "${BITRATES[@]}"; do
        for file in $RECS_PATH/*; do
            out_file=$(basename ${file} .yuv)_"$br"_"$mode"
            params="--quiet --no-progress -B $br --fps $FPS --input-res $RESOLUTION -o "$RES_PATH"/$out_file.h264 $file"
            if [ "$mode" != "none" ]; then
                params="--aq-mode 4 $params"
            fi

            start=$(gdate +%s.%3N)
            output=$($X264_PATH $params 2>&1 >/dev/null)
            end=$(gdate +%s.%3N)

            elapsed=$(echo "$end - $start" | bc)

            # extract frames encoded, fps and bitrate from output
            pattern='encoded\ ([0-9]+)\ frames,\ ([0-9.]+)\ fps,\ ([0-9.]+)\ kb/s'
            if [[ $output =~ $pattern ]]; then
                frames=${BASH_REMATCH[1]}
                fps=${BASH_REMATCH[2]}
                bitrate=${BASH_REMATCH[3]}
            else
                echo "Failed to extract values from the input string."
            fi

            echo $out_file,$elapsed,$frames,$fps,$bitrate >> results.csv
        done
    done

    # kill roi analyzer
    pkill -P $$

    sleep 1
done