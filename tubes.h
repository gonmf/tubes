#define _POSIX_C_SOURCE 199309L

#include <curses.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>

#define TRUE 1
#define FALSE 0

// Directions
#define DIR_UP 0
#define DIR_DOWN 2
#define DIR_LEFT 1
#define DIR_RIGHT 3

typedef struct __tube_{
	unsigned int x;
	unsigned int y;
	unsigned int new_dir;
	unsigned int old_dir;
	unsigned int color;
} tube_t;

#if TUBE_CROSSINGS

// Positions
#define TOP 0x1
#define LEFT 0x2
#define RIGHT 0x4
#define BOT 0x8
typedef struct __clr_{
	int mask; // positions masks
	unsigned int color; // 0 for unset
} clr_t;

#endif