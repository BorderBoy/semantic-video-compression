/*****************************************************************************
 * roi_detection.h: ROI detection
 *****************************************************************************
 * Copyright (C) 2005-2023 x264 project
 *
 * Authors: Jonas Goos
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#pragma once
#include <czmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "common/common.h"

int detect_roi(x264_t *h, x264_frame_t *frame, pixel* roi_map);
int send_planes(x264_frame_t *frame, zsock_t* requester);
void add_plane_to_message(zmsg_t *msg, x264_frame_t *frame, int index);
void destroy_roi_detection();