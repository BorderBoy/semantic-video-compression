#pragma once
#include <czmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "common/common.h"

int detect_roi(x264_frame_t *frame, pixel* roi_map);
int send_planes(x264_frame_t *frame, zsock_t* requester);
void add_plane_to_message(zmsg_t *msg, x264_frame_t *frame, int index);
void destroy_roi_detection();