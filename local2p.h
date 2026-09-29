#ifndef __DRALLY_LOCAL2P_H
#define __DRALLY_LOCAL2P_H

#include "drally.h"

/* Local 2-player (same PC, 2 SDL game controllers: BT + USB) */
void local2p_parse_args(int argc, char *argv[]);
int local2p_is_enabled(void);
int local2p_p2_idx(void);
void local2p_set_race(int my_idx, int num_cars);
int local2p_is_p2(int n);

void local2p_gamepad_init(void);
void local2p_gamepad_quit(void);

/* CTRL_* bitmask (same as race___40164h.c) for pad 0/1 */
__DWORD__ local2p_poll_pad(int pad);

/* Per-player camera save/restore for split-screen render passes */
void local2p_cam_save(int slot);
void local2p_cam_load(int slot);

#endif
