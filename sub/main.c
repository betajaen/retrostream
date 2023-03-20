#include <stdio.h>
#include <sdl3/sdl.h>

extern void NetInit();
extern void NetStop();
extern void VideoInit();
extern void VideoEventLoop();
extern void NetHeartBeat();
extern void NetStop();

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    VideoInit();
    NetInit();
    VideoEventLoop();
    NetStop();

    SDL_Quit();
    return 0;
}
