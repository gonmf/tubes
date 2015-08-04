#include "config.h"
#include "tubes.h"




static tube_t tubes[NUMBER_OF_TUBES];
static char colors = USE_COLORS;
static char color_picker;

#if TUBE_CROSSINGS
static clr_t * clrs;

static void init_clr(){
	clr_t * old = clrs;
	clrs = malloc(sizeof(clr_t) * LINES * COLS);
	int i;
	for(i = 0; i < LINES * COLS; ++i){
		clrs[i].mask = 0;
		clrs[i].color = 0;
	}
	if(old != NULL)
		free(old);
}
#endif

static void handle_winch(int sig){
	endwin();
	refresh();
	clear();
#if TUBE_CROSSINGS
	init_clr();
#endif
}

static void init_curses(){
	if(initscr() == NULL){
		fprintf(stderr, "Error: Failed to initialize ncurses.\n");
		exit(EXIT_FAILURE);
	}
	curs_set(0);
	if(colors && !has_colors())
		colors = FALSE;
	if(colors){
		start_color();
		init_pair(1,  COLOR_RED,     COLOR_BLACK);
		init_pair(2,  COLOR_GREEN,   COLOR_BLACK);
		init_pair(3,  COLOR_YELLOW,  COLOR_BLACK);
		init_pair(4,  COLOR_BLUE,    COLOR_BLACK);
		init_pair(5,  COLOR_MAGENTA, COLOR_BLACK);
		init_pair(6,  COLOR_CYAN,    COLOR_BLACK);
		init_pair(7,  COLOR_WHITE,   COLOR_BLACK);
	}
}

static void init_tubes(){
	#if TUBE_CROSSINGS
		init_clr();
	#endif
	color_picker = (rand() % 7) + 1;
	int i;
	for(i = 0; i < NUMBER_OF_TUBES; ++i){
		tubes[i].x = rand() % COLS;
		tubes[i].y = rand() % LINES;
		tubes[i].new_dir = tubes[i].old_dir = rand() % 4;
		tubes[i].color = colors ? color_picker++ : 0;
		if(color_picker > 7)
			color_picker = 1;
	}
}

static void process_tube(int t){
	int new_dir = tubes[t].new_dir;
	if((rand() % DIFICULTY_OF_SPLIT) == 0 && ((new_dir & 1) == 0 || (rand() & 1))){
		do{
			new_dir = rand() % 4;
		}while((new_dir == tubes[t].new_dir + 2) || (new_dir == tubes[t].new_dir - 2));
	}
	//fprintf(stderr, "%d\n", tubes[t].new_dir);
	switch(tubes[t].new_dir){
		case DIR_UP:
			tubes[t].y--;
			if(tubes[t].y >= LINES)
				tubes[t].y = LINES - 1;
			break;
		case DIR_DOWN:
			tubes[t].y++;
			if(tubes[t].y == LINES)
				tubes[t].y = 0;
			break;
		case DIR_LEFT:
			tubes[t].x--;
			if(tubes[t].x >= COLS)
				tubes[t].x = COLS - 1;
			break;
		case DIR_RIGHT:
			tubes[t].x++;
			if(tubes[t].x == COLS)
				tubes[t].x = 0;
			break;
	}
	tubes[t].old_dir = tubes[t].new_dir;
	tubes[t].new_dir = new_dir;
}

#if TUBE_CROSSINGS

static int to_acs(int v){
	switch(v){
		case TOP + BOT:
			return ACS_VLINE;
		case LEFT + RIGHT:
			return ACS_HLINE;
		case BOT + RIGHT:
			return ACS_ULCORNER;
		case TOP + RIGHT:
			return ACS_LLCORNER;
		case LEFT + BOT:
			return ACS_URCORNER;
		case LEFT + TOP:
			return ACS_LRCORNER;
		case TOP + RIGHT + BOT:
			return ACS_LTEE;
		case TOP + LEFT + BOT:
			return ACS_RTEE;
		case LEFT + TOP + RIGHT:
			return ACS_BTEE;
		case LEFT + BOT + RIGHT:
			return ACS_TTEE;
		//case LEFT + BOT + RIGHT + TOP:
		default:
			return ACS_PLUS;
	}
}

static int to_mask(int v){
	if(v == ACS_VLINE)
		return TOP + BOT;
	if(v == ACS_HLINE)
		return LEFT + RIGHT;
	if(v == ACS_ULCORNER)
		return BOT + RIGHT;
	if(v == ACS_LLCORNER)
		return TOP + RIGHT;
	if(v == ACS_URCORNER)
		return LEFT + BOT;
	if(v == ACS_LRCORNER)
		return LEFT + TOP;
	if(v == ACS_LTEE)
		return TOP + RIGHT + BOT;
	if(v == ACS_RTEE)
		return TOP + LEFT + BOT;
	if(v == ACS_BTEE)
		return LEFT + TOP + RIGHT;
	if(v == ACS_TTEE)
		return LEFT + BOT + RIGHT;
	return LEFT + BOT + RIGHT + TOP;
}

#endif

static void paint(int t, int c){
#if TUBE_CROSSINGS
	int x = tubes[t].x;
	int y = tubes[t].y;
	clr_t * clr = &clrs[y * COLS + x];
	if(clr->color == tubes[t].color)
		c = to_acs(clr->mask | to_mask(c));
	clr->color = tubes[t].color;
	clr->mask = to_mask(c);
#endif
	addch(c);
}

static void paint_tube2(int t){
	int a = tubes[t].old_dir;
	int b = tubes[t].new_dir;
	if((a == DIR_UP || a == DIR_DOWN) && a == b) // vertical pipe
		paint(t, ACS_VLINE);
	else
		if((a == DIR_LEFT || a == DIR_RIGHT) && a == b) // horizontal pipe
			paint(t, ACS_HLINE);
		else
			if((a == DIR_RIGHT && b == DIR_DOWN) || (a == DIR_UP && b == DIR_LEFT))
				paint(t, ACS_URCORNER);
			else
				if((a == DIR_RIGHT && b == DIR_UP) || (a == DIR_DOWN && b == DIR_LEFT))
					paint(t, ACS_LRCORNER);
				else
					if((a == DIR_UP && b == DIR_RIGHT) || (a == DIR_LEFT && b == DIR_DOWN))
						paint(t, ACS_ULCORNER);
					else
						paint(t, ACS_LLCORNER);
}

static void paint_tube(int t){
	move(tubes[t].y, tubes[t].x);
	paint_tube2(t);
}

int main(int argc, char * argv[]){
	srand(clock());
	init_curses();
	init_tubes();
	int i;


	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handle_winch;
	sigaction(SIGWINCH, &sa, NULL);

	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ANIMATION_SPEED;
	while(1){
		for(i = 0; i < NUMBER_OF_TUBES; ++i){
			process_tube(i);
			if(colors)
				attron(COLOR_PAIR(tubes[i].color));
			paint_tube(i);
		}
		refresh();
		nanosleep(&ts, NULL);
	}
	return EXIT_SUCCESS;
}
