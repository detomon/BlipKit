#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include "BKSDLTrack.h"
#include <term.h>
#include <termcap.h>
#include <ncurses.h>
#include <menu.h>
#include <sys/ioctl.h>

enum
{
	INTERACTIVE_FLAG = 1 << 0,
	DISPLAY_FLAG     = 1 << 1, // only in terminal
	PLAY_FLAG        = 1 << 2,
	CHECK_FLAG       = 1 << 3,
};

enum
{
	HELP_COMMAND,
	LOAD_COMMAND,
	QUIT_COMMAND,
	PAUSE_COMMAND,
	PLAY_COMMAND,
};

struct commandDef
{
	char const * name;
	char const * description;
	unsigned     command;
};

static const struct commandDef commands [] =
{
	{"exit", "Quit program", QUIT_COMMAND},
	{"help", "Print this screen", HELP_COMMAND},
	{"load", "Load file", LOAD_COMMAND},
	{"pause", "Pause loaded file", PAUSE_COMMAND},
	{"play", "Play loaded file", PLAY_COMMAND},
	{"quit", "Quit program", QUIT_COMMAND},
};

#define NUM_COMMAND_DEFS (sizeof (commands) / sizeof (struct commandDef))

static int          flags;
static char const * filename;
static BKSDLContext ctx;

static int getchar_nocanon (unsigned tcflags) {
	int c;
	struct termios oldtc, newtc;
	
	tcgetattr (STDIN_FILENO, & oldtc);
	
	newtc = oldtc;
	newtc.c_lflag &= ~(ICANON | ECHO | tcflags);
	
	tcsetattr (STDIN_FILENO, TCSANOW, & newtc);
	c = getchar ();
	tcsetattr (STDIN_FILENO, TCSANOW, & oldtc);
	
	return c;
}

static void printOptionHelp (void)
{
	printf (
		"usage: SDLTest [-d | --display] [-s speed | --speed speed] [-p | --play] [-r | --samplerate] file\n"
		"       SDLTest [-c | --check] file\n"
		"       SDLTest [-h | --help]\n"
	);
}

static void printInteractiveHelp (void)
{
	for (int i = 0; i < NUM_COMMAND_DEFS; i ++)
		printf ("%-8s %s\n", commands [i].name, commands [i].description);
}

static int lookupCommand (char const * key, struct commandDef const * cmd)
{
	return strcmp (key, cmd -> name);
}

static int getCommand (char const * name)
{
	struct commandDef const * cmd;
	
	cmd = bsearch (name, commands, NUM_COMMAND_DEFS, sizeof (struct commandDef), (void *) lookupCommand);
	
	if (cmd)
		return cmd -> command;
	
	return -1;
}

static int loadFile (BKSDLContext * ctx, char * const args)
{
	char const * filename = args;

	if (BKSDLContextLoadFile (ctx, filename) == 0) {
		printf ("File '%s' loaded\n", args);
	}
	else {
		fprintf (stderr, "No such file: %s\n", filename);
		return -1;
	}

	return 0;
}

static int interactiveMode (BKSDLContext * ctx)
{
	char line [1024];
	char name [64];
	char args [256];
	int command = -1;

	printf ("Entering interactive mode... (\"help\" for help, \"quit\" to quit)\n");

	while (1) {
		int c = getchar_nocanon (0);
		printf ("[%u]\n", c);
	}

	do {
		printf ("blip> ");
		fgets (line, sizeof (line), stdin);
				
		if (sscanf (line, "%63s %255[^\n]", name, args) >= 1) {
			command = getCommand (name);
			
			switch (command) {
				case QUIT_COMMAND: {
					printf ("Bye\n");
					return 0;
					break;
				}
				case HELP_COMMAND: {
					printInteractiveHelp ();
					break;
				}
				case LOAD_COMMAND: {
					SDL_LockAudio ();
					BKSDLContextUnloadData (ctx);
					loadFile (ctx, args);
					SDL_UnlockAudio ();
					break;
				}
				case PAUSE_COMMAND: {
					SDL_PauseAudio (1);
					break;
				}
				case PLAY_COMMAND: {
					SDL_PauseAudio (0);
					break;
				}
				default:
					printf ("Unknown command %s\n", name);
					break;
			}
		}
	}
	while (1);

	return 0;
}

struct option const options [] = {
	{"speed", 1, NULL, 's'},
	{"samplerate", 1, NULL, 'r'},
	{"display", 0, NULL, 'd'},
	{"help", 0, NULL, 'h'},
	{"play", 0, NULL, 'p'},
	{"check", 0, NULL, 'c'},
	{NULL, 0, NULL, 0}
};

static void fillAudio (BKSDLContext * ctx, Uint8 * stream, int len)
{
	BKUInt numChannels = ctx -> ctx.numChannels;
	BKUInt numFrames   = len / sizeof (BKFrame) / numChannels;
	
	BKContextGenerate (& ctx -> ctx, (BKFrame *) stream, numFrames);
}

static BKInt initSDL (BKSDLContext * ctx)
{
	SDL_Init (SDL_INIT_AUDIO);
	
	SDL_AudioSpec wanted;
	
	wanted.freq     = ctx -> ctx.sampleRate;
	wanted.format   = AUDIO_S16SYS;
	wanted.channels = ctx -> ctx.numChannels;
	wanted.samples  = 1024;
	wanted.callback = (void *) fillAudio;
	wanted.userdata = ctx;

	if (SDL_OpenAudio (& wanted, NULL) < 0)
		return -1;
	
	return 0;
}

static int handleOptions (BKSDLContext * ctx, int argc, const char * argv [])
{
	int    opt;
	int    longoptind = 1;
	BKUInt sampleRate = 44100;
	BKUInt speed      = 0;

	opterr = 0;

	while ((opt = getopt_long (argc, (void *) argv, "cdhps:r:", options, & longoptind)) != -1) {
		switch (opt) {
			case 's': {
				speed = atoi (optarg);
				break;
			}
			case 'd': {
				flags |= DISPLAY_FLAG;
				break;
			}
			case 'h': {
				printOptionHelp ();
				exit (0);
				break;
			}
			case 'p': {
				flags |= PLAY_FLAG;
				break;
			}
			case 'c': {
				flags |= CHECK_FLAG;
				break;
			}
			case 'r': {
				sampleRate = atoi (optarg);
				break;
			}
			default:
				fprintf (stderr, "Unknown option -- %c\n", opt);
				printOptionHelp ();
				exit (1);
				break;
		}
	}

	if (optind <= argc) {
		filename = argv [optind];
	}
	else {
		printOptionHelp ();
		exit (1);
	}

	if (BKSDLContextInit (ctx, 2, sampleRate) < 0) {
		fprintf (stderr, "Couldn't initialize context\n");
		return -1;
	}

	if (initSDL (ctx) < 0) {
		fprintf (stderr, "Couldn't initialize SDL\n");
		return -1;
	}

	// interactive mode
	if (argc == 1 || optind < argc - 1) {
		interactiveMode (ctx);
		return 1;
	}
	// play file
	else {
		SDL_LockAudio ();
		
		if (BKSDLContextLoadFile (ctx, filename) < 0) {
			fprintf (stderr, "No such file: %s\n", filename);
			exit (1);
		}
		
		if (speed) {
			for (BKInt i = 0; i < ctx -> numTracks; i ++)
				ctx -> tracks [i] -> interpreter.stepTickCount = speed;
		}

		SDL_UnlockAudio ();
	}

	return 0;
}

static BKInt handleKeys ()
{
	BKInt paused = 0;

	printf ("[space] = play/pause, [q] = stop\n");

	if (flags & PLAY_FLAG) {
		paused = 0;
		SDL_PauseAudio (0);
		printf ("\rPlaying...");
	}
	else {
		paused = 1;
		printf ("\rPaused    ");
	}

	do {
		int c = getchar_nocanon (0);
		
		switch (c) {
			case 'q': {
				printf ("\rStopped   ");
				goto end;
				break;
			}
			case ' ': {
				SDL_PauseAudio (!paused);
				paused = !paused;
				
				if (paused) {
					printf ("\rPaused    ");
				}
				else {
					printf ("\rPlaying...");
				}

				break;
			}
		}

		//SDL_Delay(100);
	}
	while (1);
	
	end:

	printf ("\n");

	return 0;
}

static void play (void)
{	
	handleKeys ();
	
	SDL_PauseAudio (1);
	SDL_CloseAudio ();
}

void handle_winch(int sig)
{
	signal (SIGWINCH, SIG_IGN);

	// Reinitialize the window to update data structures.
	endwin();
	initscr();
	refresh();
	clear();

	char tmp[128];
	sprintf(tmp, "%dx%d", COLS, LINES);

	// Approximate the center
	int x = COLS / 2 - strlen(tmp) / 2;
	int y = LINES / 2 - 1;

	mvaddstr(y, x, tmp);
	refresh();

	signal (SIGWINCH, handle_winch);
}

#define WIDTH 30
#define HEIGHT 10

int startx = 0;
int starty = 0;

char *choices[] = {
	"Choice 1",
	"Choice 2",
	"Choice 3",
	"Choice 4",
	"Exit",
};
int n_choices = sizeof(choices) / sizeof(char *);
void print_menu(WINDOW *menu_win, int highlight);

void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;
	
	x = 2;
	y = 2;
	box(menu_win, 0, 0);

	for(i = 0; i < n_choices; ++i)
	{	if(highlight == i + 1) /* High light the present choice */
	{	wattron(menu_win, A_REVERSE /*| COLOR_PAIR (1)*/);
		mvwprintw(menu_win, y, x, "%s", choices[i]);
		wattroff(menu_win, A_REVERSE);
	}
	else
		mvwprintw(menu_win, y, x, "%s", choices[i]);
		++y;
	}
	wrefresh(menu_win);
}

static void screen (void)
{
	/*struct winsize w;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);

    printf ("lines %d\n", w.ws_row);
    printf ("columns %d\n", w.ws_col);*/

	signal(SIGWINCH, handle_winch);

	WINDOW *menu_win;
	
	
		int highlight = 1;
		int choice = 0;
		int c;
		
		initscr();
		clear();
		noecho();
		cbreak();	/* Line buffering disabled. pass on everything */
		
	start_color ();
	use_default_colors ();
	
		//init_pair(1, COLOR_YELLOW, COLOR_BLUE);
		//init_pair(2, COLOR_WHITE, COLOR_BLACK);
		//bkgd (COLOR_PAIR (2));

	
	ITEM *it [5];
	MENU *me;
	
	it[0] = new_item("M1", "jkh");
	it[1] = new_item("M2", "jkh");
	it[2] = new_item("M3", "kjh");
	it[3] = new_item("Ende", "kjh");
	it[4] = 0;
	
	me = new_menu(it);
	curs_set (0);
	
	move (0, 10);
	post_menu(me);
	
	
		int startx = (80 - WIDTH) / 2;
		int starty = (24 - HEIGHT) / 2;
		
		menu_win = newwin(HEIGHT, WIDTH, starty, startx);
		keypad(menu_win, TRUE);
		mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
		refresh();
		print_menu(menu_win, highlight);
		while(1)
		{
			c = wgetch(menu_win);
			switch(c)
			{	case KEY_UP:
					menu_driver(me, REQ_UP_ITEM);
					
					if(highlight == 1)
						highlight = n_choices;
					else
						--highlight;
					break;
				case KEY_DOWN:
					menu_driver(me, REQ_DOWN_ITEM);
					
					if(highlight == n_choices)
						highlight = 1;
					else
						++highlight;
					break;
				case 10:
					choice = highlight;
					break;
				default:
					mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
					refresh();
					break;
			}
			//print_menu(menu_win, highlight);
			if(choice != 0)	/* User did a choice come out of the infinite loop */
				break;
			
			refresh ();
		}
		mvprintw(23, 0, "You chose choice %d with choice string %s\n", choice, choices[choice - 1]);
		clrtoeol();
		refresh();
		endwin();

}

#ifdef main
#undef main
#endif

int main (int argc, const char * argv [])
{
	char buf [8192];
	char * str = buf;

	setenv ("TERM", "xterm-256color", 1);

	//tgetent (buf, getenv ("TERM"));

	
	//printf ("*%ld\n", cur_term);
	//printf (">%u\n", cur_term->type.num_Strings);
	
	setupterm (NULL, 1, NULL);

//	fd_set in;
//	FD_ZERO (& in);
//	FD_SET (STDIN_FILENO, & in);
//	select (2, & in, NULL, NULL, NULL);
	
	screen ();
	
	//printf ("%s\n", cursor_down);

	//char * s = tgetstr (cursor_down, & str);
		
	//printf ("[%d %d %d %d]\n", c [0], c [1], c [2], c [3]);
	
	if (handleOptions (& ctx, argc, argv) == 0) {
		play ();
	}

    return 0;
}
