#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include "raysettings.h"
#include "crtypes.h"
#include "crscene.h"
#include "crrender.h"
#include "crtime.h"
#include "crprofile.h"

#ifdef CRSDL
#include "crsdl2_input.h"
#include "crsdl2_display.h"
#endif // CRSDL

#ifdef CRRPIFB
#include "crrpidfb_input.h"
#include "crrpidfb_display.h"
#endif // CRSDL

#pragma region Function Defs

// Funtions defs
void SignalHandler(int signum);
void Cleanup(int status);
void PopulateTestTiles(
    RaycastSettings* const settings, 
    DisplayTile* const tiles
);
bool HandleUpdateState(
    InputState* const inputState,
    RaycastSettings* const settings
);
void Update(
    const InputState* const inputState,
    float deltaMS,
    Scene* const scene,
    CycleProfile* const profile
);
void Render(
    const RaycastSettings* const settings,
    const DisplayTile* const tiles,
    const ScreenBuffer* const screen,
    const int tileCount,
    Scene* const scene,
    CycleProfile* const profile
);
void PrintSummary(
    const RaycastSettings* const settings,
    const Scene* const scene,
    const CycleProfile* const profile,
    float delta
);

#pragma endregion

// Globals
Scene* scene;
ScreenBuffer screen;

// Entry point
int main(int argc, char* argv[])
{
    scene = NULL;
    screen = DefaultScreen();
    CycleProfile profile = DefaultCycleProfile();
    InputState inputState = DefaultInputState();
    RaycastSettings settings = ParseCommandLine(argc, argv);

    signal(SIGINT, SignalHandler);

    printf("Initialising input...\n");

    if (InitInputDevice())
    {
        fprintf(stderr, "Window initialisation failed, shutting down...\n");
        Cleanup(EXIT_FAILURE);
    }

    printf("Input initialised\n");
    printf("Initialising window...\n");

    if (InitDisplay(&settings.screenFormat, &screen))
    {
        fprintf(stderr, "Window initialisation failed, shutting down...\n");
        Cleanup(EXIT_FAILURE);
    }

    printf("Window initialised\n");
    printf("Initialising scene...\n");

    DisplayTile tiles[3];
    PopulateTestTiles(&settings, tiles);
    scene = 
        CreateTestScene(
            &settings.playerSettings, 
            settings.wallHeight, 
            80.0f
        );
    
    printf("Scene initialised\n");
    printf("Starting main loop...\n");
    
    bool isRunning = true;
    float delta;
    float period = 1.0f / (float)settings.targetFps;
    float targetInterval = period * 1000.0f;
    uint64_t currentTicks = GetTicks();
    uint64_t previousTicks = currentTicks;

    while (isRunning)
    {
        currentTicks = GetTicks();
        delta = GetTimeInMS(currentTicks - previousTicks);

        UpdateInputState(&inputState);

        if (HandleUpdateState(&inputState, &settings))
        {
            printf("Received QUIT event from input\n");
            isRunning = false;
            continue;
        }

        if (delta > targetInterval)
        {
            Update(
                &inputState, 
                delta,
                scene, 
                &profile
            );

            Render(
                &settings,
                tiles,
                &screen,
                3,
                scene,
                &profile
            );

            PrintSummary(
                &settings,
                scene,
                &profile,
                delta
            );

            ResetProfile(&profile);

            previousTicks = currentTicks;
        }
    }
    
    printf("Closing down...\n");

    Cleanup(EXIT_SUCCESS);
}

#pragma region Function Bodies

// Function bodies
void SignalHandler(int signum)
{
    if (signum == SIGTERM ||
        signum == SIGABRT ||
        signum == SIGINT)
    {
        Cleanup(EXIT_SUCCESS);
    }
}

void Cleanup(int status)
{
    if (scene != NULL)
    {
        CleanupScene(scene);
    }
    DestroyInputDevice();
    DestroyDisplay(&screen);
    exit(status);
}

void PopulateTestTiles(
    RaycastSettings* const settings,
    DisplayTile* const tiles)
{
    DisplayTile staticSceneTile =
    {
        .tileType = StaticScene,
        .borderColor = CreateColorRGB(255, 255, 0),
        .viewport =
        {
            .x = settings->screenFormat.width * 0.05,
            .y = settings->screenFormat.height * 0.55,
            .w = settings->screenFormat.width * 0.4,
            .h = settings->screenFormat.height * 0.4
        }
    };

    DisplayTile staticPlayerTile =
    {
        .tileType = StaticPlayer,
        .borderColor = CreateColorRGB(0, 255, 255),
        .viewport =
        {
            .x = settings->screenFormat.width * 0.55,
            .y = settings->screenFormat.height * 0.55,
            .w = settings->screenFormat.width * 0.4,
            .h = settings->screenFormat.height * 0.4
        }
    };

    DisplayTile firstPersonTile =
    {
        .tileType = FirstPerson,
        .borderColor = CreateColorRGB(255, 0, 255),
        .viewport =
        {
            .x = settings->screenFormat.width * 0.2,
            .y = settings->screenFormat.height * 0.05,
            .w = settings->screenFormat.width * 0.6,
            .h = settings->screenFormat.height * 0.45
        }
    };

    tiles[0] = staticSceneTile;
    tiles[1] = staticPlayerTile;
    tiles[2] = firstPersonTile;
}

bool HandleUpdateState(
    InputState* const inputState,
    RaycastSettings* const settings)
{
    if (inputState->quit)
    {
        return true;
    }

    if (inputState->toggleDebug)
    {
        settings->printDebugInfo = !settings->printDebugInfo;
    }

    if (inputState->toggleRenderMode)
    {
        if (settings->renderMode == FullStaticScene)
        {
            settings->renderMode = FullFirstPerson;
        }
        else if (settings->renderMode == FullFirstPerson)
        {
            settings->renderMode = Tiled;
        }
        else if (settings->renderMode == Tiled)
        {
            settings->renderMode = FullStaticScene;
        }
    }

    return false;
}

void Update(
    const InputState* const inputState,
    float deltaMS,
    Scene* const scene,
    CycleProfile* const profile)
{
    UpdatePlayerPosition(
        scene, 
        deltaMS,
        inputState, 
        profile
    );
}

void Render(
    const RaycastSettings* const settings,
    const DisplayTile* const tiles,
    const ScreenBuffer* const screen,
    const int tileCount,
    Scene* const scene,
    CycleProfile* const profile)
{
    uint64_t renderStartTime = GetTicks();

    if (settings->renderMode == FullStaticScene)
    {
        scene->camera =
            (Frame2D){
                .position =
                {
                    .x = screen->width / 2,
                    .y = screen->height / 2
                },
                .theta = 0.0f
        };
        RenderSceneTopDown(screen, scene, profile);
    }
    else if (settings->renderMode == FullFirstPerson)
    {
        scene->camera =
            (Frame2D){
                .position =
                {
                    .x = screen->width / 2,
                    .y = screen->height / 2
                },
                .theta = 0.0f
        };
        RenderSceneFirstPerson(screen, scene, profile);
    }
    else if (settings->renderMode == Tiled)
    {
        RenderTiles(screen, scene, tiles, tileCount, profile);
    }

    uint64_t presentStartTime = GetTicks();
    RenderDisplay(screen);
    profile->renderPresentTimeMS = GetTimeInMS(GetTicks() - presentStartTime);
    profile->totalRenderTimeMS = GetTimeInMS(GetTicks() - renderStartTime);
}

void PrintSummary(
    const RaycastSettings* const settings,
    const Scene* const scene,
    const CycleProfile* const profile,
    float delta)
{
    uint64_t printStartTime = GetTicks();

    if (settings->printDebugInfo)
    {
        printf("\033[2J");
        printf("\033[H");
        printf(
            "Player: %f, %f, %f\n",
            scene->player.frame.position.x,
            scene->player.frame.position.y,
            scene->player.frame.theta
        );
        printf("Frame time:\t%f ms\n", delta);
        PrintProfileStats(profile);
        printf("Print time:\t%f ms\n", GetTimeInMS(GetTicks() - printStartTime));
    }
}

#pragma endregion
