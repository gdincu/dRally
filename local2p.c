#include "local2p.h"
#include "drally_keyboard.h"

#define L2P_CTRL_NULL 0x00
#define L2P_CTRL_ACCELERATE 0x01
#define L2P_CTRL_BRAKE 0x02
#define L2P_CTRL_STEER_LEFT 0x04
#define L2P_CTRL_STEER_RIGHT 0x08
#define L2P_CTRL_TURBO_BOOST 0x10
#define L2P_CTRL_MACHINE_GUN 0x20
#define L2P_CTRL_DROP_MINE 0x40
#define L2P_CTRL_HORN 0x42

int g_local2p_enabled = 0;
static int g_p2_idx = 1;

/* camera slots for split-screen: 0 = P1, 1 = P2 */
static int cam_X[2], cam_Y[2], cam_XInc[2], cam_YInc[2], cam_TLX[2], cam_TLY[2];
static int cam_init = 0;

extern int X___243c8ch;
extern int Y___243c90h;
extern int X_Inc;
extern int Y_Inc;
extern int TRX_VIEWPORT_TL_X;
extern int TRX_VIEWPORT_TL_Y;
extern __BYTE__ kmap[];

static SDL_GameController *pads[2] = {0, 0};

void local2p_parse_args(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--2p") || !strcmp(argv[i], "-2p") ||
            !strcmp(argv[i], "--local2p") || !strcmp(argv[i], "--two-player")) {
            g_local2p_enabled = 1;
        }
    }
}

int local2p_is_enabled(void) { return g_local2p_enabled; }
int local2p_p2_idx(void) { return g_p2_idx; }

void local2p_set_race(int my_idx, int num_cars) {
    if (!g_local2p_enabled) return;
    /* P2 = next slot after P1, wraps. Race guarantees >= 2 cars (see race_main). */
    if (num_cars >= 2) {
        g_p2_idx = (my_idx + 1) % num_cars;
        if (g_p2_idx == my_idx) g_p2_idx = (my_idx + 1) % num_cars;
    } else {
        g_p2_idx = 1;
    }
    cam_init = 0;
}

int local2p_is_p2(int n) {
    return g_local2p_enabled && (n == g_p2_idx);
}

void local2p_gamepad_init(void) {
    int i, n;
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("[local2p] SDL joystick init failed: %s\n", SDL_GetError());
        return;
    }
    n = 0;
    for (i = 0; i < SDL_NumJoysticks() && n < 2; i++) {
        if (!SDL_IsGameController(i)) continue;
        pads[n] = SDL_GameControllerOpen(i);
        if (pads[n]) {
            printf("[local2p] pad%d: %s (%s)\n", n,
                SDL_GameControllerName(pads[n]),
                (SDL_JoystickGetAttached(SDL_GameControllerGetJoystick(pads[n])) ? "attached" : "?"));
            n++;
        }
    }
    if (n == 0) {
        printf("[local2p] no SDL game controllers found (BT+USB). Keyboard fallback: P1 arrows/A/Z, P2 W/S/Q/E.\n");
    } else if (n == 1) {
        printf("[local2p] only 1 controller found; P2 falls back to keyboard (W/S/Q/E).\n");
    } else {
        printf("[local2p] 2 controllers ready. --2p split-screen: top=P1 bottom=P2.\n");
    }
}

void local2p_gamepad_quit(void) {
    int i;
    for (i = 0; i < 2; i++) {
        if (pads[i]) {
            SDL_GameControllerClose(pads[i]);
            pads[i] = 0;
        }
    }
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
}

static int axis_left(int pad) {
    Sint16 v = SDL_GameControllerGetAxis(pads[pad], SDL_CONTROLLER_AXIS_LEFTX);
    if (v < -8000) return 1;
    return 0;
}
static int axis_right(int pad) {
    Sint16 v = SDL_GameControllerGetAxis(pads[pad], SDL_CONTROLLER_AXIS_LEFTX);
    if (v > 8000) return 1;
    return 0;
}

__DWORD__ local2p_poll_pad(int pad) {
    __DWORD__ f = L2P_CTRL_NULL;
    static int prev_mine[2] = {0, 0};
    int mine_now;
    Sint16 trigR, trigL;

    if (pad < 0 || pad > 1 || !pads[pad] || !SDL_GameControllerGetAttached(pads[pad]))
        return 0;

    if (axis_left(pad) || SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_DPAD_LEFT))
        f |= L2P_CTRL_STEER_LEFT;
    if (axis_right(pad) || SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
        f |= L2P_CTRL_STEER_RIGHT;

    trigR = SDL_GameControllerGetAxis(pads[pad], SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    trigL = SDL_GameControllerGetAxis(pads[pad], SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    if (trigR > 8000 || SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_A))
        f |= L2P_CTRL_ACCELERATE;
    if (trigL > 8000 || SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_X))
        f |= L2P_CTRL_BRAKE;
    if (SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_A))
        f |= L2P_CTRL_TURBO_BOOST;
    if (SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_B))
        f |= L2P_CTRL_MACHINE_GUN;
    mine_now = SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_Y);
    if (mine_now && !prev_mine[pad])
        f |= L2P_CTRL_DROP_MINE;
    prev_mine[pad] = mine_now;
    if (SDL_GameControllerGetButton(pads[pad], SDL_CONTROLLER_BUTTON_START))
        f |= L2P_CTRL_HORN;

    if ((f & L2P_CTRL_BRAKE) && (f & L2P_CTRL_DROP_MINE)) f &= ~L2P_CTRL_BRAKE;
    if (f & L2P_CTRL_TURBO_BOOST) f |= L2P_CTRL_ACCELERATE;
    return f;
}

void local2p_cam_save(int slot) {
    if (slot < 0 || slot > 1) return;
    cam_X[slot] = X___243c8ch;
    cam_Y[slot] = Y___243c90h;
    cam_XInc[slot] = X_Inc;
    cam_YInc[slot] = Y_Inc;
    cam_TLX[slot] = TRX_VIEWPORT_TL_X;
    cam_TLY[slot] = TRX_VIEWPORT_TL_Y;
}

void local2p_cam_load(int slot) {
    if (slot < 0 || slot > 1) return;
    if (!cam_init) {
        /* first use: both slots start from current shared state */
        cam_X[0] = cam_X[1] = X___243c8ch;
        cam_Y[0] = cam_Y[1] = Y___243c90h;
        cam_XInc[0] = cam_XInc[1] = X_Inc;
        cam_YInc[0] = cam_YInc[1] = Y_Inc;
        cam_TLX[0] = cam_TLX[1] = TRX_VIEWPORT_TL_X;
        cam_TLY[0] = cam_TLY[1] = TRX_VIEWPORT_TL_Y;
        cam_init = 1;
    }
    X___243c8ch = cam_X[slot];
    Y___243c90h = cam_Y[slot];
    X_Inc = cam_XInc[slot];
    Y_Inc = cam_YInc[slot];
    TRX_VIEWPORT_TL_X = cam_TLX[slot];
    TRX_VIEWPORT_TL_Y = cam_TLY[slot];
}
