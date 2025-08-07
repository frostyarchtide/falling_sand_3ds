#include "sandbox.hpp"

#include <3ds.h>
#include <citro2d.h>

#define TOP_WIDTH      400
#define BOTTOM_WIDTH   320
#define SCREEN_HEIGHT  240

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

    Sandbox sandbox(UVec2 { TOP_WIDTH, SCREEN_HEIGHT }, 8);

    while (aptMainLoop())
    {
        hidScanInput();

        u32 kDown = hidKeysDown();

        if (kDown & KEY_START)
            break;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32f(0.1f, 0.1f, 0.1f, 1.0f));
        C2D_SceneBegin(top);

        sandbox.update();
        sandbox.draw();

        C3D_FrameEnd(0);
    }

    cfguExit();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
}
