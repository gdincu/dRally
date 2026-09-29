#include "drally.h"
#include "drally_display.h"

#if defined(DR_MULTIPLAYER)
extern __DWORD__ ___19bd60h;
void ___623d4h(void);
#endif

extern void_cb ___2432c8h;

void dRally_Keyboard_init(void);
void ___60466h(int, int);
void ___3e720h(void);
void __VGA3_SETMODE(void);
void dRally_System_init(void);
void dRally_Sound_quit(void);
void dRally_System_clean(void);
void local2p_parse_args(int argc, char *argv[]);
void local2p_gamepad_init(void);
void local2p_gamepad_quit(void);

static void ___10060h(void){

	printf("\nDeath Rally *** Full Version 1.1\n");
}

static void ___100dch(void){

	__VGA3_SETMODE();
	printf("DEATH RALLY Exit: CTRL+ALT+DEL pressed!\n");
	exit(0x70);
}

int main(int argc, char * argv[]){

	int i;
	local2p_parse_args(argc, argv);
	for(i = 1; i < argc; i++){
		if(!strcmp(argv[i], "--fullscreen") || !strcmp(argv[i], "-f") ||
		   !strcmp(argv[i], "--full-screen") || !strcmp(argv[i], "-fullscreen"))
			dRally_Display_setFullscreen(1);
		if(!strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w"))
			dRally_Display_setFullscreen(0);
	}
	dRally_System_init();
#if defined(DR_LETTERBOX)
	dRally_Display_init(W_LETTERBOX);
#else
	dRally_Display_init(W_SHRINK);
#endif // DR_LETTERBOX
	___10060h();
	___60466h(70, 1);
	___2432c8h = &___100dch;
	dRally_Keyboard_init();
	local2p_gamepad_init();
	___3e720h();

#if defined(DR_MULTIPLAYER)
	if(___19bd60h != 0) ___623d4h();
#endif

	dRally_Sound_quit();
	dRally_Display_clean();
	local2p_gamepad_quit();
	dRally_System_clean();

	return 0;
}
