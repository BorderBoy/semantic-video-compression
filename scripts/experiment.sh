#!/bin/bash
X264_PATH="../x264/x264"
RECS_PATH=./recordings
RES_PATH=./results
ROI_ANALYZER_PATH=../roi-analyzer/build
BITRATES=(750 1500) #in kbit/s
ROI_MODES=("none" "face" "fes")
FPS=30
RESOLUTION="1280x720"
REPEATS=3 # number of times each encoding is done
RES_PATTERN='encoded\ ([0-9]+)\ frames,\ ([0-9.]+)\ fps,\ ([0-9.]+)\ kb/s'
FILE_PATTERN='.+_([0-9x]+)_([0-9]+)'

# clear results file
echo "name,encoding time,encoded frames,fps,bitrate" > $RES_PATH/results.csv

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
            for (( i=0; i<$REPEATS; i++ )); do   
                base_name=$(basename $file .yuv)
                
                if [[ $base_name =~ $FILE_PATTERN ]]; then
                    RESOLUTION=${BASH_REMATCH[1]}
                    FPS=${BASH_REMATCH[2]}
                else
                    echo "Failed to fps and resolution from filename. Using default values."
                fi

                out_file="$base_name"_"$br"_"$mode"
                params="--quiet --no-progress --profile baseline --vbv-bufsize $((4*$br)) --bitrate $br --fps $FPS --input-res $RESOLUTION -o "$RES_PATH"/$out_file.h264 $file"
                if [ "$mode" != "none" ]; then
                    params="--aq-mode 4 $params"
                fi

                start=$(gdate +%s.%3N)
                output=$($X264_PATH $params 2>&1 >/dev/null)
                end=$(gdate +%s.%3N)

                elapsed=$(echo "$end - $start" | bc)

                # extract frames encoded, fps and bitrate from output
                if [[ $output =~ $RES_PATTERN ]]; then
                    frames=${BASH_REMATCH[1]}
                    fps=${BASH_REMATCH[2]}
                    bitrate=${BASH_REMATCH[3]}
                else
                    echo "Failed to extract values from x264 output string."
                fi

                echo $out_file,$elapsed,$frames,$fps,$bitrate >> $RES_PATH/results.csv
                echo "Done: $out_file, $elapsed s, $frames frames, $fps fps, $bitrate kbit/s"
            done
        done
    done

    # kill roi analyzer
    pkill -P $$

    sleep 1
done