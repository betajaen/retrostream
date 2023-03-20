#include <stdlib.h>
#include <stdbool.h>
#include <zmq.h>
#include <pthread.h>
#include <unistd.h>

extern void VideoStart();
extern void VideoStop();
extern void VideoFrame(void* sock);

typedef struct FrameHeader {
    uint8_t  magic[4];
    uint32_t length;
    uint32_t fmt;
    uint32_t num;
} FrameHeader;

volatile bool sub_alive;
volatile bool video_alive;
volatile int video_timer;

#define MSG_HEARTBEAT   0xDD
#define MSG_QUIT        0xFF

void* req(void* ctx) {

    int rv;
    void* rep;

    rep = zmq_socket(ctx, ZMQ_REP);
    zmq_bind(rep, "tcp://*:41200");

    char req[4] = { 0 };

    while(sub_alive == true) {

        int rv_size = zmq_recv(rep, req, sizeof(req), 0);

        if (rv_size <= 0) {
            printf("Message error %s\n", zmq_strerror(errno));
            continue;
        }

        req[3] = 0xFF;
        zmq_send(rep, req, 4, 0);

        switch(req[0]) {
            case MSG_HEARTBEAT:
                video_timer = 60;

                if (video_alive == false) {
                    video_alive = true;
                    VideoStart();
                }
                break;
            case MSG_QUIT:
                sub_alive = false;
            break;
        }

    }

    zmq_close(rep);
    return NULL;
}

void NetSend(void* sock, uint32_t num, uint32_t fmt, void* data, uint32_t length) {
    FrameHeader hdr;
    hdr.magic[0] = 0xCA;
    hdr.magic[1] = 0xAF;
    hdr.magic[2] = 0xBE;
    hdr.magic[3] = 0xEF;
    hdr.num = num;
    hdr.fmt = fmt;
    hdr.length = length;

    zmq_send(sock, &hdr, sizeof(hdr), ZMQ_SNDMORE);
    zmq_send(sock, data, length, 0);
}

void pub(void* ctx) {
    uint64_t bufSize;
    void* sock = zmq_socket(ctx, ZMQ_PUB);
    bufSize = 800*600*3 + 1024;
    zmq_setsockopt(sock, ZMQ_SNDBUF, (void*) &bufSize, sizeof(bufSize));
    zmq_bind(sock, "tcp://*:10000");

    while(sub_alive == true) {

        video_timer--;

        if (video_timer <= 0) {
            if (video_alive == true) {
                printf("Heartbeat stopped. Stopping video.\n");
                video_alive = false;
                VideoStop();
            }
            usleep(500000);
            continue;
        }

        VideoFrame(sock);
    }

    zmq_close(sock);
}

void NetPublisher(const char* subEp, const char* repEp) {
    void* ctx = zmq_ctx_new();

    sub_alive = true;
    video_alive = false;

    pthread_t req_thread;
    pthread_create(&req_thread, NULL, req, ctx);

    pub(ctx);
    zmq_ctx_shutdown(ctx);
}
