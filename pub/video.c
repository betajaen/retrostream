#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define TEST 1

extern void NetSend(void* sock, uint32_t num, uint32_t fmt, void* data, uint32_t dataLength);
void VideoStart();
void VideoStop();
void VideoFrame(void* sock);

#if TEST == 0

#define VIDEO_WIDTH  800
#define VIDEO_HEIGHT 600
#define VIDEO_DEPTH   24
#define VIDEO_BPP    (VIDEO_DEPTH / 8)
#define VIDEO_BUFFER_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT * VIDEO_BPP)

static uint8_t* sBuffer = NULL;
static uint32_t sFrameNum = 0;
static uint32_t sFrameFmt = 0;

void VideoStart() {
    sFrameNum = 1;
    sFrameFmt = 1;
    sBuffer = malloc(VIDEO_BUFFER_SIZE);
    srand((int) sBuffer & 0xFFFFFFF);
    printf("Video Start\n");
}

void VideoStop() {
    printf("Video Stop\n");
    if (sBuffer != NULL) {
        free(sBuffer);
        sBuffer = 0;
    }
    sFrameNum = 0;
    sFrameFmt = 0;
}

int fr = 0;

void VideoFrame(void* sock) {
    usleep(16667); // approx. 1/60 of second

    uint8_t* buf = sBuffer;

    for(uint32_t i=0;i < VIDEO_BUFFER_SIZE;i++) {
        *buf++ = (fr + i) & 0xFF;
    }
    fr++;
    NetSend(sock, sFrameNum, sFrameFmt, (void*) sBuffer, VIDEO_BUFFER_SIZE);
}

#else

#include <linux/videodev2.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

static int       sVideoFd;
static uint32_t  sFrameNum = 0;
static uint32_t  sFrameFmt = 0;
static int       sBufferSize = 0;
static uint8_t*  sVideoBuffer;

void initVideo() {
    // https://medium.com/@athul929/capture-an-image-using-v4l2-api-5b6022d79e1d
    sVideoFd = open("/dev/video0", O_RDWR | O_NONBLOCK, 0);

    struct v4l2_format format = {0};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 800;
    format.fmt.pix.height = 600;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
    format.fmt.pix.field = V4L2_FIELD_NONE;
    int res = ioctl(sVideoFd, VIDIOC_S_FMT, &format);
    if(res == -1) {
        perror("Could not set format");
        exit(1);
    }
}

void stopVideo() {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(sVideoFd, VIDIOC_STREAMON, &type) == -1){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    close(sVideoFd);
}

int requestVideoBuffer(int fd, int count) {
    struct v4l2_requestbuffers req = {0};
    req.count = count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        exit(1);
    }
    return 0;
}

int queryVideoBuffer(int fd) {
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    int res = ioctl(fd, VIDIOC_QUERYBUF, &buf);
    if(res == -1) {
        perror("Could not query buffer");
        return 2;
    }
    sVideoBuffer = (u_int8_t*)mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    return buf.length;
}

int startVideoBuffer(int fd) {
    unsigned int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_STREAMON, &type) == -1){
        perror("VIDIOC_STREAMON");
        exit(1);
    }
}

int queueVideoBuffer(int fd) {
    struct v4l2_buffer bufd = {0};
    bufd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufd.memory = V4L2_MEMORY_MMAP;
    bufd.index = 0;
    if(-1 == ioctl(fd, VIDIOC_QBUF, &bufd))
    {
        perror("VIDIOC_QBUF");
        return 1;
    }
    return bufd.bytesused;
}

int unqueueVideoBuffer(int fd) {
    struct v4l2_buffer bufd = {0};
    bufd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufd.memory = V4L2_MEMORY_MMAP;
    bufd.index = 0;
    if(-1 == ioctl(fd, VIDIOC_DQBUF, &bufd))
    {
        perror("VIDIOC_DQBUF");
        return 1;
    }
    return bufd.bytesused;
}

void VideoStart() {
    sFrameNum = 1;
    sFrameFmt = 1;

    initVideo();
    requestVideoBuffer(sVideoFd, 1);
    sBufferSize = queryVideoBuffer(sVideoFd);
    startVideoBuffer(sVideoFd);
}

void VideoStop() {
    printf("Video Stop\n");
    stopVideo();
    sFrameNum = 0;
    sFrameFmt = 0;
}

int fr = 0;

void VideoFrame(void* sock) {

    queueVideoBuffer(sVideoFd);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sVideoFd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 2; //set timeout to 2 second
    int r = select(sVideoFd + 1, &fds, NULL, NULL, &tv);
    if(-1 == r){
        perror("Waiting for Frame");
        exit(1);
    }

    // printf("Sending frame %ud of size %ud", sFrameNum, sBufferSize);

    NetSend(sock, sFrameNum, sFrameFmt, (void*) sVideoBuffer, sBufferSize);
    //send(sock, &info, sizeof(FrameInfo), 0);
    //send(sock, sVideoBuffer, bufferSize, 0);

    sFrameNum++;

    unqueueVideoBuffer(sVideoFd);

}
#endif