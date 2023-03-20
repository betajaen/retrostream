#include <sdl3/sdl.h>
#include <stdbool.h>
#include <string.h>

static SDL_Window* sWindow;
static SDL_Renderer* sRenderer;
static bool sAlive;
static uint32_t sVideoWidth, sVideoHeight;
static SDL_Texture* sFrameBuffers[2] = { NULL, NULL };
static uint32_t sReadBuffer = 0, sWriteBuffer = 1;

extern void NetHeartBeat();
int NetStartVideoFeed(void*);

extern void* BeginWriteFrame(uint32_t fmt, uint32_t length) {
    if (sAlive == false)
        return NULL;
    SDL_Texture* tex = sFrameBuffers[sWriteBuffer];
    void* pixels = NULL;
    int pitch;
    int rv = SDL_LockTexture(tex, NULL, (void**) &pixels, &pitch);
    if (rv != 0) {
        printf("Could not lock  texture!\n");
        return NULL;
    }
    return pixels;
}

extern void EndWriteFrame() {

    if (sAlive == false) {
        return;
    }

    SDL_Texture* tex = sFrameBuffers[sWriteBuffer];
    SDL_UnlockTexture(tex);

    SDL_FRect  srcRect;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = sVideoWidth;
    srcRect.h = sVideoHeight;

    SDL_FRect dstRect;
    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.w = sVideoWidth;
    dstRect.h = sVideoHeight;

    //int rv = SDL_RenderTexture(sRenderer, tex, NULL, NULL);
    SDL_RenderTextureRotated(sRenderer, tex, &srcRect, &dstRect, 0, NULL, SDL_FLIP_VERTICAL);
    SDL_RenderPresent(sRenderer);

    sReadBuffer = 1 - sReadBuffer;
    sWriteBuffer = 1 - sWriteBuffer;
}


static void ClearFrame() {
    const uint32_t size = sVideoWidth*sVideoHeight*3;
    uint8_t* px = (uint32_t*) BeginWriteFrame(0, size);
    memset(px, 0x88, size);
    EndWriteFrame();
}


void VideoInit() {

    sAlive = false;

    sVideoWidth = 800;
    sVideoHeight = 600;

    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);

    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "nearest");
    sWindow = SDL_CreateWindow("RetroStream", sVideoWidth / mode->display_scale, sVideoHeight / mode->display_scale, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_METAL);
    sRenderer = SDL_CreateRenderer(sWindow, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderLogicalPresentation(sRenderer, sVideoWidth, sVideoHeight, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE, SDL_SCALEMODE_NEAREST);

    for(uint32_t i=0;i < 2;i++) {
        sFrameBuffers[i] = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, sVideoWidth, sVideoHeight);
    }

    //SDL_SetRenderDrawColor(sRenderer, 255, 0, 255, 0);
    //SDL_RenderClear(sRenderer);
    //SDL_RenderPresent(sRenderer);
}

static void VideoTeardown() {

    if (sFrameBuffers[0]) {
        SDL_DestroyTexture(sFrameBuffers[0]);
    }

    if (sFrameBuffers[1]) {
        SDL_DestroyTexture(sFrameBuffers[1]);
    }

    if (sRenderer) {
        SDL_DestroyRenderer(sRenderer);
    }

    if (sWindow) {
        SDL_DestroyWindow(sWindow);
    }

}

void VideoEventLoop() {
    SDL_Event evt;

    sAlive = true;

    SDL_Thread* thread = SDL_CreateThread(NetStartVideoFeed, "RetroStreamVideoSub", &sAlive);

    uint64_t hb = SDL_GetTicks();

    while(sAlive) {
        SDL_PollEvent(&evt);
        if (evt.type == SDL_EVENT_QUIT) {
            sAlive = false;
        }
        if (evt.type == SDL_EVENT_KEY_UP) {
            SDL_KeyCode kc = evt.key.keysym.sym;

            if (kc == SDLK_ESCAPE) {
                sAlive = false;
            }
        }

        uint64_t now = SDL_GetTicks();
        if ((now - hb) > 100) {
            hb = now;
            NetHeartBeat();
        }
        SDL_Delay(16);
    }

    int threadStatus;
    SDL_WaitThread(thread, &threadStatus);

    VideoTeardown();
}