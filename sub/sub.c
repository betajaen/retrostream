#include <stdlib.h>
#include <stdbool.h>
#include <czmq.h>

#define SUB_EP "tcp://192.168.1.212:10000"
#define REQ_EP "tcp://192.168.1.212:41200"

extern void* BeginWriteFrame(uint32_t fmt, uint32_t length);
extern void EndWriteFrame();

typedef struct FrameHeader {
    uint8_t  magic[4];
    uint32_t length;
    uint32_t fmt;
    uint32_t num;
} FrameHeader;

void* ctx = NULL;

static void req(uint8_t msgKind) {
    void* sock = zmq_socket(ctx, ZMQ_REQ);
    zmq_connect(sock, REQ_EP);

    zmq_msg_t msg;
    zmq_msg_init_size(&msg, 4);
    uint8_t* data = (void*) zmq_msg_data(&msg);
    data[0] = msgKind;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
    zmq_msg_send(&msg, sock, 0);

    zmq_close(sock);
}

void NetHeartBeat() {
    req(0xDD);
}

void NetVideoStop() {
    req(0xFF);
}

void sub(bool* abort) {

    uint64_t bufSize = 0;
    int rv;
    void* sock;
    _connect:

    printf("Attempting to Connect\n");
    sock = zmq_socket(ctx, ZMQ_SUB);
    zmq_setsockopt(sock, ZMQ_SUBSCRIBE, "", 0);
    bufSize = 800*600*3 + 1024;
    zmq_setsockopt(sock, ZMQ_RCVBUF, (void*) &bufSize, sizeof(bufSize));
    zmq_connect(sock, SUB_EP);
    printf("Connect\n");

    while(*abort) {
        FrameHeader hdr;

        rv = zmq_recv(sock, &hdr, sizeof(hdr), 0);

        if (rv <= 0) {
            zmq_close(sock);
            goto _connect;
        }

        if (hdr.magic[0] != 0xCA && hdr.magic[1] != 0xAF && hdr.magic[2] != 0xBE && hdr.magic[3] != 0xEF) {
            printf("Bad Magic!!");
            zmq_close(sock);
            goto _connect;
        }

        void* fr = BeginWriteFrame(hdr.fmt, hdr.length);
        if (fr == NULL) {
            zmq_close(sock);
            goto _connect;
        }

        rv = zmq_recv(sock, fr, hdr.length, 0);
        if (rv <= 0) {
            zmq_close(sock);
            goto _connect;
        }
        EndWriteFrame();
    }
}

void NetInit() {
    ctx = zmq_ctx_new();
}

void NetStop() {
    zmq_ctx_shutdown(ctx);
    ctx = NULL;
}

int NetStartVideoFeed(void* abortBool) {
    sub(abortBool);
}