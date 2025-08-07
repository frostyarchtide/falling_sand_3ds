#include <3ds.h>
#include <RIP/C3D.h>
#include <citro2d.h>

#define TOP_WIDTH      400
#define BOTTOM_WIDTH   320
#define SCREEN_HEIGHT  240
#define TEXTURE_WIDTH  256
#define TEXTURE_HEIGHT 128
#define TEXTURE_SIZE   TEXTURE_WIDTH * TEXTURE_HEIGHT
#define TEXTURE_SCALE  2
#define LOGICAL_WIDTH  TOP_WIDTH / TEXTURE_SCALE
#define LOGICAL_HEIGHT SCREEN_HEIGHT / TEXTURE_SCALE

struct UVec2 {
    size_t x;
    size_t y;
};

int main()
{
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    cfguInit();
    consoleInit(GFX_BOTTOM, 0);

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    u32* data = (u32*) linearAlloc(TEXTURE_SIZE * 4);
    u32* new_data = (u32*) linearAlloc(TEXTURE_SIZE * 4);
    
    for (size_t y = 0; y < LOGICAL_HEIGHT; y++) {
        for (size_t x = 0; x < LOGICAL_WIDTH; x++) {
            size_t i = y * TEXTURE_WIDTH + x;

            if (rand() % 4 == 0) {
                data[i] = 0xFFFFFFFF;
            } else {
                data[i] = 0x000000FF;
            }
        }
    }

    C3D_Tex texture;
    C3D_TexInit(&texture, TEXTURE_WIDTH, TEXTURE_HEIGHT, GPU_RGBA8);

    ripConvertAndLoadC3DTexImage(&texture, data, (GPU_TEXFACE) 0, 0);

    Tex3DS_SubTexture subtexture = {
        TEXTURE_WIDTH,
        TEXTURE_HEIGHT,
        0.0f, 1.0f, 1.0f, 0.0f
    };

    C2D_Image image;
    image.tex = &texture;
    image.subtex = &subtexture;

    while (aptMainLoop())
    {
        hidScanInput();

        u32 kDown = hidKeysDown();

        if (kDown & KEY_START)
            break;

        memcpy(new_data, data, TEXTURE_SIZE * 4);
        for (size_t y = 0; y < LOGICAL_HEIGHT; y++) {
            for (size_t x = 0; x < LOGICAL_WIDTH; x++) {
                size_t i = y * TEXTURE_WIDTH + x;
                if (data[i] != 0xFFFFFFFF) continue;

                size_t down_i = i + TEXTURE_WIDTH;
                size_t down_left_i = down_i - 1;
                size_t down_right_i = down_i + 1;

                if (y < LOGICAL_HEIGHT - 1 && new_data[down_i] == 0x000000FF) {
                    new_data[down_i] = 0xFFFFFFFF;
                    new_data[i] = 0x000000FF;
                } else if (x > 0 && new_data[down_left_i] == 0x000000FF) {
                    new_data[down_left_i] = 0xFFFFFFFF;
                    new_data[i] = 0x000000FF;
                } else if (x < LOGICAL_WIDTH - 1 && new_data[down_right_i] == 0x000000FF) {
                    new_data[down_right_i] = 0xFFFFFFFF;
                    new_data[i] = 0x000000FF;
                }
            }
        }
        memcpy(data, new_data, TEXTURE_SIZE * 4);

        ripConvertAndLoadC3DTexImage(&texture, data, (GPU_TEXFACE) 0, 0);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32f(0.1f, 0.1f, 0.1f, 1.0f));
        C2D_SceneBegin(top);

        C2D_DrawImageAt(image, 0.0f, 0.0f, 0.0f, nullptr, TEXTURE_SCALE, TEXTURE_SCALE);

        C3D_FrameEnd(0);
    }

    linearFree(data);
    cfguExit();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
}
