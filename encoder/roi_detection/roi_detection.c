#include "roi_detection.h"

int detect_roi(x264_frame_t *frame, pixel* roi_map){
    static zsock_t* requester = NULL;

    if (requester == NULL){
        printf ("Connecting server...\n");
        requester = zsock_new (ZMQ_REQ);
        zsock_connect (requester, "tcp://localhost:5555");
    }

    // Send planes
    sendPlanes(frame, requester);

    // Receive response
    zframe_t *message = zframe_recv (requester);

    printf ("Received: %d bytes\n", zframe_size (message));

    zframe_destroy (&message);

    zsock_destroy (&requester);

    return 0;
}

int sendPlanes(x264_frame_t *frame, zsock_t* requester){
    // Send image
    zmsg_t *msg = zmsg_new();

    addPlaneToMessage(msg, frame, 0);
    addPlaneToMessage(msg, frame, 1);

    int error = zmsg_send(&msg, requester);

    if(error != 0){
        return -1;
    }

    return 0;
}

void addPlaneToMessage(zmsg_t *msg, x264_frame_t *frame, int index){
    printf ("Sending plane %d, height: %d, width: %d, stride: %d\n", index, frame->i_lines[index], frame->i_width[index], frame->i_stride[index]);
    zframe_t* image_lines = zframe_new(&frame->i_lines[index], sizeof(int));
    zframe_t* image_width = zframe_new(&frame->i_width[index], sizeof(int));
    zframe_t* image_stride = zframe_new(&frame->i_stride[index], sizeof(int));

    bool interleaved = index == 1;
    zframe_t* image_interleaved = zframe_new(&interleaved, sizeof(bool));

    zframe_t* image = zframe_new(frame->plane[index], frame->i_stride[index] * frame->i_lines[index]);

    zmsg_append(msg, &image_lines);
    zmsg_append(msg, &image_width);
    zmsg_append(msg, &image_stride);
    zmsg_append(msg, &image_interleaved);
    zmsg_append(msg, &image);
}