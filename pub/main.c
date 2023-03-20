#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// apt-get install libczmq-dev
extern void NetPublisher(const char* ep);

int main(int argc, char** argv) {
    printf("RetroStream Publisher\n");
    NetPublisher("tcp://0.0.0.0:10000");
    return 0;
}
