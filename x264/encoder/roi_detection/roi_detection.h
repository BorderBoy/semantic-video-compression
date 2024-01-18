#pragma once
#include <czmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "common/common.h"

void connectSocket();
int detect_roi(x264_frame_t *frame, pixel* roi_map);
int sendPlanes(x264_frame_t *frame, zsock_t* requester);
void addPlaneToMessage(zmsg_t *msg, x264_frame_t *frame, int index);