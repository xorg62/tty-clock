/*		clock.c	 - TTY-Clock main function
 *
 *		Copyright © 2008 Duquesnoy Martin <xorg62@gmail.com>
 *
 *		This program is free software; you can redistribute it and/or modify
 *		it under the terms of the GNU General Public License as published by
 *		the Free Software Foundation; either version 2 of the License, or
 *		(at your option) any later version.
 *
 *		This program is distributed in the hope that it will be useful,
 *		but WITHOUT ANY WARRANTY; without even the implied warranty of
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *		GNU General Public License for more details.
 *
 *		You should have received a copy of the GNU General Public License
 *		along with this program; if not, write to the Free Software
 *		Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *		MA 02110-1301, USA.
 */

#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <getopt.h>	 

#define printh() printf("tty-clock usage : tty-clock -[option] -[option] <arg>\n\n\
  -s, --second		 Show seconds\n\
  -t, --tw		 Set the hour in 12h format\n\
  -x  <integer>		 Set the clock to X\n\
  -y  <integer>		 Set the clock to Y\n\
  -v, --version		 Show tty-clock version\n\
  -i, --info		 Show some info about tty-clock\n\
  -h, --help		 Show this page\n\n\
Try keypad arrow for move the clock :-)\n\
push S for enable the second and T for enable the 12H hours format.\n");\
							 

#define LGNUM 30
#define XLENGTH 5
#define YLENGTH 52
#define DEPTHB -1

/* *************** */
/* BIG NUMBER INIT */
/* *************** */

static const char number[10][LGNUM] = {
	 {1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1}, /* 0 */
	 {0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1}, /* 1 */
	 {1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1}, /* 2 */
	 {1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1}, /* 3 */
	 {1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1}, /* 4 */
	 {1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1}, /* 5 */
	 {1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1}, /* 6 */
	 {1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1}, /* 7 */
	 {1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1}, /* 8 */
	 {1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1}  /* 9 */
};

static struct option long_options[] ={
	{"help",	 0, NULL, 'h'},
	{"version", 0, NULL, 'v'},
	{"info",	 0, NULL, 'i'},
	{"second",  0, NULL, 's'},
	{"twelve",	0, NULL, 't'},
	{NULL,		0, NULL, 0}
};

typedef struct {
	bool second;
	bool twelve;
} OPTIONS;

OPTIONS option;

typedef struct {
	unsigned int hour[2];
	unsigned int minute[2];
	unsigned int second[2];
	unsigned int month_day;
	unsigned int month;
	unsigned int year;
}  TIME;

TIME sdate;

char *meridiem;

int SCHANGE = 19;
int maxcol;
int maxlin;
int temp_dp;
int defx=1;
int defy=1;
int bg = COLOR_BLACK;

struct tm *tm;
time_t lt;

/* ***************** */
/* STARTING FUNCTION */
/* ***************** */

void 
start(void) {
	initscr ();	 
	noecho ();
	keypad (stdscr, TRUE);	 
	start_color ();
	refresh();
	if (use_default_colors() == OK);
	bg = -1;
	init_pair(1,COLOR_BLACK, COLOR_GREEN);
	init_pair(2, bg, bg);
	init_pair(3,COLOR_GREEN, bg);
	curs_set(0);
}

/* **************************** */
/* BIG NUMBER PRINTING FUNCTION */
/* **************************** */

void 
print_number(int num, int x, int y) {
	int i,u,count=0;
	char c;
	int tab[LGNUM];
	int lx=x;
	int ly=y;
	 
	for (u = 0; u < LGNUM; ++u){
		tab[u] = number[num][u];
	}
	

	for (i = 0; i < LGNUM; ++i) {
		c = (tab[i] != 1) ? 2 : 1;
  
		if(count == 6){ 
			++lx;
			ly=y;
			count = 0;
		}

		move(lx, ly);
		attron(COLOR_PAIR(c));
		addch(' ');
		attroff(COLOR_PAIR(c));
		++ly;
		++count;
	}
}

/* ******************************** */
/* ARRANGE FINAL POSITION OF NUMBER */
/* ******************************** */

void  
arrange_clock(int h1, int h2, 
			  int m1, int m2, 
			  int s1, int s2) {
	int i;

	temp_dp = (option.second) ? 21 : 12;

	print_number(h1, defx, defy);
	print_number(h2, defx, defy + 7);
	 
	attron(COLOR_PAIR(1));
	
	move(defx + 1, defy + 15);
	printw("%s",meridiem);
	move(defx + 3, defy + 15);
	printw("  ");

	attroff(COLOR_PAIR(1));

	print_number(m1, defx, defy + 19);
	print_number(m2, defx, defy + 26);

	if(option.second){
		
		 attron(COLOR_PAIR(1));
	     mvaddstr(defx + 1, defy + 34,"  ");
		 mvaddstr(defx + 3, defy + 34,"  ");
		 attroff(COLOR_PAIR(1));
	 
		 print_number(s1, defx, defy + 38);
		 print_number(s2, defx, defy + 45);
	 }

	for(i = defy + DEPTHB; i < defy + YLENGTH - SCHANGE; ++i){
		 mvaddch(defx + DEPTHB, i, ACS_HLINE);
		 mvaddch(defx + XLENGTH, i,ACS_HLINE);
	}

	for (i = defx + DEPTHB; i < defx + XLENGTH; ++i){
		 mvaddch(i, defy + DEPTHB, ACS_VLINE);
		 mvaddch(i, defy + YLENGTH - SCHANGE, ACS_VLINE);
	}
	 
	mvaddch(defx + DEPTHB, defy + DEPTHB, ACS_ULCORNER);
	mvaddch(defx + XLENGTH, defy + DEPTHB, ACS_LLCORNER);
	mvaddch(defx + DEPTHB, defy + YLENGTH - SCHANGE, ACS_URCORNER);
	mvaddch(defx + XLENGTH, defy + YLENGTH - SCHANGE, ACS_LRCORNER);

	
	move(defx + XLENGTH + 1,defy + temp_dp);
 	attron(COLOR_PAIR(3));
	printw("%d/%d/%d",sdate.month_day,sdate.month,sdate.year);
	attroff(COLOR_PAIR(3));

}


/* ********************* */
/* KEY CHECKING FUNCTION */
/* ********************* */

void
check_key(void) {
	int c;
	c = getch();
	switch(c) {
		case KEY_UP:
		case 'k':
		case 'K':
			if(defx > 1) 
				--defx;
				clear();
			break;
		case KEY_DOWN:
		case 'j':
		case 'J':
			if(defx + XLENGTH + 2 < maxcol) 
				++defx;
				clear();
			break;
		case KEY_LEFT:
		case 'h':
		case 'H':
			if(defy > 1) 
				--defy;
				clear();
			break;
		case KEY_RIGHT:	 
		case 'l':
		case 'L':
			if(defy + YLENGTH - SCHANGE + 1 < maxlin) 
				++defy;
				clear();
			break;
		case 's':
		case 'S':
			if(!option.second ){
				SCHANGE = 0;
				clear();
				option.second = 1;
			} else {
				SCHANGE = 19;
				clear();
				option.second = 0;
			}
			break;
		case 't':
		case 'T':
			if(!option.twelve) {
				clear();
				option.twelve = 1;
			} else {
				clear();
				option.twelve = 0;
			}
			break;
		case 'q':
		case 'Q':
			endwin();
			exit(EXIT_SUCCESS);
			break;
		}
}

/* ********************* */
/* GETTING TIME FUNCTION */
/* ********************* */

void
get_time(void) {
	int ihour;
	tm = localtime(&lt);
	lt = time(NULL);

	ihour = tm->tm_hour;
	
	if (option.twelve && ihour > 12) {
		meridiem = "PM";
	} else if (option.twelve && ihour < 12) {
		meridiem = "AM";
	} else {
		meridiem = "  ";
	}

	ihour = (option.twelve && ihour > 12) ? ihour - 12 : ihour;
	ihour = (option.twelve && !ihour) ? 12 : ihour;


	sdate.hour[0] = ihour / 10;
	sdate.hour[1] = ihour % 10;

	sdate.minute[0] = tm->tm_min / 10;
	sdate.minute[1] = tm->tm_min % 10;

	sdate.month_day = tm->tm_mday;
	sdate.month = tm->tm_mon + 1;
	sdate.year = tm->tm_year + 1900;

	if(option.second) {
		sdate.second[0] = tm->tm_sec / 10;
		sdate.second[1] = tm->tm_sec % 10;
	}
}

/* *********** */
/* RUN FUCTION */
/* *********** */

void 
run(void) {
	get_time();
	arrange_clock(sdate.hour[0], sdate.hour[1],
					sdate.minute[0], sdate.minute[1],
					sdate.second[0], sdate.second[1]);
	maxcol = getmaxy(stdscr);
	maxlin = getmaxx(stdscr);
	refresh();
	halfdelay(1);
}

/* ************ */
/* MAIN FUCTION */
/* ************ */

int 
main(int argc,char **argv) {
	int c;

	static struct option long_options[] ={
		{"help",	 0, NULL, 'h'},
		{"version", 0, NULL, 'v'},
		{"info",	 0, NULL, 'i'},
		{"second",  0, NULL, 's'},
		{"twelve",	0, NULL, 't'},
		{NULL,		0, NULL, 0}
		};

	  while ((c = getopt_long(argc,argv,"tx:y:vsih",
			long_options,NULL)) != -1) {
		switch(c) {
			case 'h':
			default:
				 printh();
				 exit(EXIT_SUCCESS);
				 break;
			case 'i':
				 printf("TTY-Clock ,Martin Duquesnoy© (xorg62@gmail.com)\n");
				 exit(EXIT_SUCCESS);
				 break;
			case 'v':
				 printf("TTY-Clock v0.1.4\n");
				 exit(EXIT_SUCCESS);
				 break;
			case 'x':
				 if(atoi(optarg)<0){
					  exit(EXIT_FAILURE);
				 } else {
					  defx = atoi(optarg) + 1;
				 }
				 break;
			case 'y':
				 if(atoi(optarg)<0){
					  exit(EXIT_FAILURE);
				 } else {
					  defy = atoi(optarg) + 1;
				 }
				 break;
			case 's':
				 SCHANGE = 0;
				 option.second = 1;
				 break;
			case 't':
				option.twelve = 1;
				break;
			}
		}

	 start();

/* first time init */	 

	run();

 /* endless loop */

	while(1) {
		usleep(10000);
		check_key();
		run();
	}
	 endwin();
	 return 0;
}
