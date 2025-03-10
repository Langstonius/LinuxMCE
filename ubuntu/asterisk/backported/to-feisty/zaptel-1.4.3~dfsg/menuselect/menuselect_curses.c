/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2005 - 2006, Russell Bryant
 *
 * Russell Bryant <russell@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*
 * \file
 *
 * \author Russell Bryant <russell@digium.com>
 * 
 * \brief curses frontend for selection maintenance
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <curses.h>

#include "menuselect.h"

#define MENU_TITLEBAR	"*************************************"
#define MENU_HELP	"Press 'h' for help."

#define TITLE_HEIGHT	7

#define MIN_X		80
#define MIN_Y		21

#define PAGE_OFFSET	10

extern int changes_made;

/*! Maximum number of characters horizontally */
static int max_x = 0;
/*! Maximum number of characters vertically */
static int max_y = 0;

static const char * const help_info[] = {
	"scroll              => up/down arrows",
	"toggle selection    => Enter",
	"select              => y",
	"deselect            => n",
	"select all          => F8",
	"deselect all        => F7",
	"back                => left arrow",
	"quit                => q",
	"save and quit       => x",
	"",
	"XXX means dependencies have not been met",
	"    or a conflict exists",
	"",
	"< > means a dependency has been deselected",
	"    and will be automatically re-selected",
	"    if this item is selected",
	"",
	"( ) means a conflicting item has been",
	"    selected",
};

/*! \brief Handle a window resize in xterm */
static void winch_handler(int sig)
{
	getmaxyx(stdscr, max_y, max_x);

	if (max_x < MIN_X || max_y < MIN_Y) {
		fprintf(stderr, "Terminal must be at least %d x %d.\n", MIN_X, MIN_Y);
		max_x = MIN_X - 1;
		max_y = MIN_Y - 1;
	}
}

/*! \brief Handle a SIGQUIT */
static void sigint_handler(int sig)
{

}

/*! \brief Display help information */
static void show_help(WINDOW *win)
{
	int i;

	wclear(win);
	for (i = 0; i < (sizeof(help_info) / sizeof(help_info[0])); i++) {
		wmove(win, i, max_x / 2 - 15);
		waddstr(win, help_info[i]);
	}
	wrefresh(win);
	getch(); /* display the help until the user hits a key */
}

static int really_quit(WINDOW *win)
{
	int c;
	wclear(win);
	wmove(win, 2, max_x / 2 - 15);
	waddstr(win, "ARE YOU SURE?");
        wmove(win, 3, max_x / 2 - 12);
	waddstr(win, "--- It appears you have made some changes, and");
	wmove(win, 4, max_x / 2 - 12);
	waddstr(win, "you have opted to Quit without saving these changes!");
	wmove(win, 6, max_x / 2 - 12);
	waddstr(win, "  Please Enter Y to exit without saving;");
	wmove(win, 7, max_x / 2 - 12);
	waddstr(win, "  Enter N to cancel your decision to quit,");
	wmove(win, 8, max_x / 2 - 12);
	waddstr(win, "     and keep working in menuselect, or");
	wmove(win, 9, max_x / 2 - 12);
	waddstr(win, "  Enter S to save your changes, and exit");
	wmove(win, 10, max_x / 2 - 12);
	wrefresh(win);
	while ((c=getch())) {
		if (c == 'Y' || c == 'y') {
			c = 'q';
			break;
		}
		if (c == 'S' || c == 's') {
			c = 'S';
			break;
		}
		if (c == 'N' || c == 'n') {
			c = '%';
			break;
		}
	}
	return c;
}

static void draw_main_menu(WINDOW *menu, int curopt)
{
	struct category *cat;
	char buf[64];
	int i = 0;

	wclear(menu);

	AST_LIST_TRAVERSE(&categories, cat, list) {
		wmove(menu, i++, max_x / 2 - 10);
		if (!strlen_zero(cat->displayname))
			snprintf(buf, sizeof(buf), "%d.%s %s", i, i < 10 ? " " : "", cat->displayname);
		else
			snprintf(buf, sizeof(buf), "%d.%s %s", i, i < 10 ? " " : "", cat->name);
		waddstr(menu, buf);
	}

	wmove(menu, curopt, (max_x / 2) - 15);
	waddstr(menu, "--->");
	wmove(menu, 0, 0);

	wrefresh(menu);
}

static void display_mem_info(WINDOW *menu, struct member *mem, int start, int end)
{
	char buf[64];
	struct depend *dep;
	struct conflict *con;
	struct use *use;

	wmove(menu, end - start + 2, max_x / 2 - 16);
	wclrtoeol(menu);
	wmove(menu, end - start + 3, max_x / 2 - 16);
	wclrtoeol(menu);
	wmove(menu, end - start + 4, max_x / 2 - 16);
	wclrtoeol(menu);
	wmove(menu, end - start + 5, max_x / 2 - 16);
	wclrtoeol(menu);

	if (mem->displayname) {
		wmove(menu, end - start + 2, max_x / 2 - 16);
		waddstr(menu, mem->displayname);
	}
	if (!AST_LIST_EMPTY(&mem->deps)) {
		wmove(menu, end - start + 3, max_x / 2 - 16);
		strcpy(buf, "Depends on: ");
		AST_LIST_TRAVERSE(&mem->deps, dep, list) {
			strncat(buf, dep->name, sizeof(buf) - strlen(buf) - 1);
			strncat(buf, dep->member ? "(M)" : "(E)", sizeof(buf) - strlen(buf) - 1);
			if (AST_LIST_NEXT(dep, list))
				strncat(buf, ", ", sizeof(buf) - strlen(buf) - 1);
		}
		waddstr(menu, buf);
	}
	if (!AST_LIST_EMPTY(&mem->uses)) {
		wmove(menu, end - start + 4, max_x / 2 - 16);
		strcpy(buf, "Can use: ");
		AST_LIST_TRAVERSE(&mem->uses, use, list) {
			strncat(buf, use->name, sizeof(buf) - strlen(buf) - 1);
			if (AST_LIST_NEXT(use, list))
				strncat(buf, ", ", sizeof(buf) - strlen(buf) - 1);
		}
		waddstr(menu, buf);
	}
	if (!AST_LIST_EMPTY(&mem->conflicts)) {
		wmove(menu, end - start + 5, max_x / 2 - 16);
		strcpy(buf, "Conflicts with: ");
		AST_LIST_TRAVERSE(&mem->conflicts, con, list) {
			strncat(buf, con->name, sizeof(buf) - strlen(buf) - 1);
			strncat(buf, con->member ? "(M)" : "(E)", sizeof(buf) - strlen(buf) - 1);
			if (AST_LIST_NEXT(con, list))
				strncat(buf, ", ", sizeof(buf) - strlen(buf) - 1);
		}
		waddstr(menu, buf);
	}

}

static void draw_category_menu(WINDOW *menu, struct category *cat, int start, int end, int curopt, int changed)
{
	int i = 0;
	int j = 0;
	struct member *mem;
	char buf[64];

	if (!changed) {
		/* If all we have to do is move the cursor, 
		 * then don't clear the screen and start over */
		AST_LIST_TRAVERSE(&cat->members, mem, list) {
			i++;
			if (curopt + 1 == i) {
				display_mem_info(menu, mem, start, end);
				break;
			}
		}
		wmove(menu, curopt - start, max_x / 2 - 9);
		wrefresh(menu);
		return;
	}

	wclear(menu);

	i = 0;
	AST_LIST_TRAVERSE(&cat->members, mem, list) {
		if (i < start) {
			i++;
			continue;
		}
		wmove(menu, j++, max_x / 2 - 10);
		i++;
		if ((mem->depsfailed == HARD_FAILURE) || (mem->conflictsfailed == HARD_FAILURE)) {
			snprintf(buf, sizeof(buf), "XXX %d.%s %s", i, i < 10 ? " " : "", mem->name);
		} else if (mem->depsfailed == SOFT_FAILURE) {
			snprintf(buf, sizeof(buf), "<%s> %d.%s %s", mem->enabled ? "*" : " ", i, i < 10 ? " " : "", mem->name);
		} else if (mem->conflictsfailed == SOFT_FAILURE) {
			snprintf(buf, sizeof(buf), "(%s) %d.%s %s", mem->enabled ? "*" : " ", i, i < 10 ? " " : "", mem->name);
		} else {
			snprintf(buf, sizeof(buf), "[%s] %d.%s %s", mem->enabled ? "*" : " ", i, i < 10 ? " " : "", mem->name);
		}
		waddstr(menu, buf);
		
		if (curopt + 1 == i)
			display_mem_info(menu, mem, start, end);

		if (i == end)
			break;
	}

	wmove(menu, curopt - start, max_x / 2 - 9);
	wrefresh(menu);
}

static void play_space(void);

static int run_category_menu(WINDOW *menu, int cat_num)
{
	struct category *cat;
	int i = 0;
	int start = 0;
	int end = max_y - TITLE_HEIGHT - 6;
	int c;
	int curopt = 0;
	int maxopt;
	int changed = 1;

	AST_LIST_TRAVERSE(&categories, cat, list) {
		if (i++ == cat_num)
			break;
	}
	if (!cat)
		return -1;

	maxopt = count_members(cat) - 1;

	draw_category_menu(menu, cat, start, end, curopt, changed);

	while ((c = getch())) {
		changed = 0;
		switch (c) {
		case KEY_UP:
			if (curopt > 0) {
				curopt--;
				if (curopt < start) {
					start--;
					end--;
					changed = 1;
				}
			}
			break;
		case KEY_DOWN:
			if (curopt < maxopt) {
				curopt++;
				if (curopt > end - 1) {
					start++;
					end++;
					changed = 1;
				}
			}
			break;
		case KEY_NPAGE:
			/* XXX Move down the list by PAGE_OFFSET */
			break;
		case KEY_PPAGE:
			/* XXX Move up the list by PAGE_OFFSET */
			break;
		case KEY_LEFT:
		case 27:	/* Esc key */
			return 0;
		case KEY_RIGHT:
		case KEY_ENTER:
		case '\n':
		case ' ':
			toggle_enabled_index(cat, curopt);
			changed = 1;
			break;
		case 'y':
		case 'Y':
			set_enabled(cat, curopt);
			changed = 1;
			break;
		case 'n':
		case 'N':
			clear_enabled(cat, curopt);
			changed = 1;
			break;
		case 'h':
		case 'H':
			show_help(menu);
			changed = 1;
			break;
		case KEY_F(7):
			set_all(cat, 0);
			changed = 1;
			break;
		case KEY_F(8):
			set_all(cat, 1);
			changed = 1;
		default:
			break;	
		}
		if (c == 'x' || c == 'X' || c == 'Q' || c == 'q')
			break;	
		draw_category_menu(menu, cat, start, end, curopt, changed);
	}

	wrefresh(menu);

	return c;
}

static void draw_title_window(WINDOW *title)
{
	wclear(title);
	wmove(title, 1, (max_x / 2) - (strlen(MENU_TITLEBAR) / 2));
	waddstr(title, MENU_TITLEBAR);
	wmove(title, 2, (max_x / 2) - (strlen(menu_name) / 2));
	waddstr(title, menu_name);
	wmove(title, 3, (max_x / 2) - (strlen(MENU_TITLEBAR) / 2));
	waddstr(title, MENU_TITLEBAR);
	wmove(title, 5, (max_x / 2) - (strlen(MENU_HELP) / 2));
	waddstr(title, MENU_HELP);
	wrefresh(title);
}

int run_menu(void)
{
	WINDOW *title;
	WINDOW *menu;
	int maxopt;
	int curopt = 0;
	int c;
	int res = 0;

	initscr();
	getmaxyx(stdscr, max_y, max_x);
	signal(SIGWINCH, winch_handler); /* handle window resizing in xterm */
	signal(SIGINT, sigint_handler); /* handle window resizing in xterm */

	if (max_x < MIN_X || max_y < MIN_Y) {
		fprintf(stderr, "Terminal must be at least %d x %d.\n", MIN_X, MIN_Y);
		endwin();
		return -1;
	}

	cbreak(); /* don't buffer input until the enter key is pressed */
	noecho(); /* don't echo user input to the screen */
	keypad(stdscr, TRUE); /* allow the use of arrow keys */
	clear();
	refresh();

	maxopt = count_categories() - 1;
	
	/* We have two windows - the title window at the top, and the menu window gets the rest */
	title = newwin(TITLE_HEIGHT, max_x, 0, 0);
	menu = newwin(max_y - TITLE_HEIGHT, max_x, TITLE_HEIGHT, 0);
	draw_title_window(title);	
	draw_main_menu(menu, curopt);
	
	while ((c = getch())) {
		switch (c) {
		case KEY_UP:
			if (curopt > 0)
				curopt--;
			break;
		case KEY_DOWN:
			if (curopt < maxopt)
				curopt++;
			break;
		case KEY_RIGHT:
		case KEY_ENTER:
		case '\n':
		case ' ':
			c = run_category_menu(menu, curopt);
			break;
		case 'h':
		case 'H':
			show_help(menu);
			break;
		case 'i':
		case 'I':
			play_space();
			draw_title_window(title);
		default:
			break;	
		}
		if (c == 'q' || c == 'Q' || c == 27 || c == 3) {
			if (changes_made) {
				c = really_quit(menu);
				if (c == 'q') {
					res = -1;
					break;
				}
			} else {
				res = -1;
				break;
			}
		}
		if (c == 'x' || c == 'X' || c == 's' || c == 'S')
			break;	
		draw_main_menu(menu, curopt);
	}

	endwin();

	return res;
}

enum blip_type {
	BLIP_TANK = 0,
	BLIP_SHOT,
	BLIP_BOMB,
	BLIP_ALIEN
};

struct blip {
	enum blip_type type;
	int x;
	int y;
	int goingleft;
	AST_LIST_ENTRY(blip) entry;
};

static AST_LIST_HEAD_NOLOCK(, blip) blips;

static int score = 0;
static int num_aliens = 0;
struct blip *tank = NULL;

/*! Probability of a bomb, out of 100 */
#define BOMB_PROB   1

static int init_blips(void)
{
	int i, j;
	struct blip *cur;

	srandom(time(NULL) + getpid());

	/* make tank */
	cur = calloc(1, sizeof(struct blip));
	if (!cur)
		return -1;
	cur->type = BLIP_TANK;
	cur->x = max_x / 2;
	cur->y = max_y - 1;
	AST_LIST_INSERT_HEAD(&blips, cur, entry);
	tank = cur;

	/* 3 rows of 10 aliens */
	num_aliens = 0;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 10; j++) {
			cur = calloc(1, sizeof(struct blip));
			if (!cur)
				return -1;
			cur->type = BLIP_ALIEN;
			cur->x = (j * 2) + 1;
			cur->y = (i * 2) + 1;
			AST_LIST_INSERT_HEAD(&blips, cur, entry);
			num_aliens++;
		}
	}

	return 0;
}

static inline chtype type2chtype(enum blip_type type)
{
	switch (type) {
	case BLIP_TANK:
		return 'A';
	case BLIP_ALIEN:
		return 'X';
	case BLIP_SHOT:
		return '|';
	case BLIP_BOMB:
		return 'o';
	default:
		break;
	}
	return '?';
}

static int repaint_screen(void)
{
	struct blip *cur;

	clear();

	wmove(stdscr, 0, 0);
	wprintw(stdscr, "Score: %d", score);

	AST_LIST_TRAVERSE(&blips, cur, entry) {
		wmove(stdscr, cur->y, cur->x);
		waddch(stdscr, type2chtype(cur->type));	
	}

	wmove(stdscr, 0, max_x - 1);

	wrefresh(stdscr);

	return 0;
}

static int tank_move_left(void)
{
	if (tank->x > 0)
		tank->x--;
	
	return 0;
}

static int tank_move_right(void)
{
	if (tank->x < (max_x - 1))
		tank->x++;

	return 0;
}

static int count_shots(void)
{
	struct blip *cur;
	int count = 0;

	AST_LIST_TRAVERSE(&blips, cur, entry) {
		if (cur->type == BLIP_SHOT)
			count++;
	}

	return count;
}

static int tank_shoot(void)
{
	struct blip *shot;

	if (count_shots() == 3)
		return 0;

	score--;

	shot = calloc(1, sizeof(struct blip));
	if (!shot)
		return -1;
	shot->type = BLIP_SHOT;
	shot->x = tank->x;
	shot->y = max_y - 2;
	AST_LIST_INSERT_HEAD(&blips, shot, entry);

	return 0;
}

static int move_aliens(void)
{
	struct blip *cur;

	AST_LIST_TRAVERSE(&blips, cur, entry) {
		if (cur->type != BLIP_ALIEN) {
			/* do nothing if it's not an alien */
			continue;
		}
		if (cur->goingleft && (cur->x == 0)) {
			cur->y++;
			cur->goingleft = 0;
		} else if (!cur->goingleft && cur->x == (max_x - 1)) {
			cur->y++;
			cur->goingleft = 1;
		} else if (cur->goingleft) {
			cur->x--;
		} else {
			cur->x++;
		}
		/* Alien into the tank == game over */
		if (cur->x == tank->x && cur->y == tank->y)
			return 1;
		if (random() % 100 < BOMB_PROB && cur->y != max_y) {
			struct blip *bomb = calloc(1, sizeof(struct blip));
			if (!bomb)
				continue;
			bomb->type = BLIP_BOMB;
			bomb->x = cur->x;
			bomb->y = cur->y + 1;
			AST_LIST_INSERT_HEAD(&blips, bomb, entry);
		}
	}

	return 0;
}

static int move_bombs(void)
{
	struct blip *cur;

	AST_LIST_TRAVERSE(&blips, cur, entry) {
		if (cur->type != BLIP_BOMB)
			continue;
		cur->y++;
		if (cur->x == tank->x && cur->y == tank->y)
			return 1;
	}

	return 0;
}

static void move_shots(void)
{
	struct blip *cur;

	AST_LIST_TRAVERSE(&blips, cur, entry) {
		if (cur->type != BLIP_SHOT)
			continue;
		cur->y--;
	}
}

static int remove_blip(struct blip *blip)
{
	if (!blip)
		return -1;

	AST_LIST_REMOVE(&blips, blip, entry);

	if (blip->type == BLIP_ALIEN)
		num_aliens--;

	free(blip);

	return 0;	
}

static void game_over(int win)
{
	clear();

	wmove(stdscr, max_y / 2, max_x / 2 - 10);
	wprintw(stdscr, "Game over!  You %s!", win ? "win" : "lose");

	wmove(stdscr, 0, max_x - 1);

	wrefresh(stdscr);

	sleep(1);

	while (getch() != ' ');

	return;
}

static int check_shot(struct blip *shot)
{
	struct blip *cur;
	
	AST_LIST_TRAVERSE(&blips, cur, entry) {
		if (cur->type != BLIP_ALIEN)
			continue;
		if (cur->x == shot->x && cur->y == shot->y) {
			score += 20;
			remove_blip(shot);
			remove_blip(cur);
			if (!num_aliens) {
				game_over(1);
				return 1;
			}
		}
	}

	return 0;
}

static int check_placement(void)
{
	struct blip *cur;

	AST_LIST_TRAVERSE_SAFE_BEGIN(&blips, cur, entry) {
		if (cur->y <= 0 || cur->y >= max_y) {
			AST_LIST_REMOVE_CURRENT(&blips, entry);
			remove_blip(cur);
		} else if (cur->type == BLIP_SHOT && check_shot(cur))
			return 1;
	}
	AST_LIST_TRAVERSE_SAFE_END

	return 0;
}

static void play_space(void)
{
	int c;
	unsigned int jiffies = 1;
	int quit = 0;
	struct blip *blip;

	clear();
	nodelay(stdscr, TRUE);
	init_blips();
	repaint_screen();

	for (;;) {
		c = getch();
		switch (c) {
		case ' ':
			tank_shoot();
			break;
		case KEY_LEFT:
			tank_move_left();
			break;
		case KEY_RIGHT:
			tank_move_right();
			break;
		case 'x':
		case 'X':
		case 'q':
		case 'Q':
			quit = 1;
		default:
			/* ignore unknown input */
			break;
		}
		if (quit)
			break;
		if (!(jiffies % 25)) {
			if (move_aliens() || move_bombs()) {
				game_over(0);
				break;
			}
			if (check_placement())
				break;
		}
		if (!(jiffies % 10)) {
			move_shots();
			if (check_placement())
				break;
		}
		repaint_screen();
		jiffies++;
		usleep(1000);
	}

	while ((blip = AST_LIST_REMOVE_HEAD(&blips, entry)))
		free(blip);

	nodelay(stdscr, FALSE);
}
