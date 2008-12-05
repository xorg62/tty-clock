/*
 *      clock.c TTY-CLOCK Main file.
 *      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of the  nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <getopt.h>

#define printh() printf("tty-clock usage : tty-clock -[option] -[option] <arg>\n\n\
  -s, --second		 Show seconds\n                                 \
  -b, --block        	 Lock the keyboard\n                            \
  -c, --center		 Set the clock at the center of the terminal\n  \
  -t, --tw		 Set the hour in 12h format\n                   \
  -x  <integer>		 Set the clock to X\n                           \
  -y  <integer>		 Set the clock to Y\n                           \
  -v, --version		 Show tty-clock version\n                       \
  -i, --info		 Show some info about tty-clock\n               \
  -h, --help		 Show this page\n\n                             \
Try keypad arrow for move the clock :-)\n                               \
push S for enable the second and T for enable the 12H hours format.\n");\

#define LGNUM   30
#define DIFFSEC 19
#define DEPTHB  -1
#define MAXW    getmaxx(stdscr)
#define MAXH    getmaxy(stdscr)

typedef enum { False, True } Bool;

void start(void);
void check_key(Bool);
void get_time(void);
void set_center(void);
void run(void);

/* BIG NUMBER INIT */
static const Bool number[][LGNUM] =
{
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

/* VARIABLE INIT */
static struct option long_options[] =
{
     {"help",    0, NULL, 'h'},
     {"version", 0, NULL, 'v'},
     {"info",	 0, NULL, 'i'},
     {"second",  0, NULL, 's'},
     {"twelve",  0, NULL, 't'},
     {"block",	 0, NULL, 'b'},
     {"center",	 0, NULL, 'c'},
     {NULL,	 0, NULL, 0}
};

typedef struct
{
     Bool second;
     Bool twelve;
     Bool keylock;
} option_t;

typedef struct
{
     int x;
     int y;
     int width;
     int height;
} geo_t;

typedef struct
{
     unsigned int hour[2];
     unsigned int minute[2];
     unsigned int second[2];
     unsigned int month_day;
     unsigned int month;
     unsigned int year;
} date_t;

option_t option = { False, False, True };
geo_t geo = {1, 1, 33, 5}; /* Base position of the clock */
Bool fixcenter = False;
date_t sdate;
char *meridiem;
int temp_dp;
int bg;


struct tm *tm;
time_t lt;

/* STARTING FUNCTION */
void
start(void)
{
     initscr();
     noecho();
     keypad(stdscr, TRUE);
     start_color();
     refresh();
     bg = (use_default_colors() == OK) ? -1 : COLOR_BLACK;
     init_pair(1, COLOR_BLACK, COLOR_GREEN);
     init_pair(2, bg, bg);
     init_pair(3, COLOR_GREEN, bg);
     curs_set(0);
     clear();
}

/* BIG NUMBER PRINTING FUNCTION */
void
print_number(int num, int x, int y)
{
     int i, u, count = 0;
     int tab[LGNUM];
     int lx = x, ly = y;
     char c;

     for(u = 0; u < LGNUM; ++u)
          tab[u] = number[num][u];

     for(i = 0; i < LGNUM; ++i)
     {
          c = (tab[i] != 1) ? 2 : 1;

          if(count == 6)
          {
               ++lx;
               ly = y;
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

/* ARRANGE FINAL POSITION OF NUMBER */
void
arrange_clock(int h1, int h2,
              int m1, int m2,
              int s1, int s2)
{
     int i;

     temp_dp = (option.second) ? 21 : 12;

     print_number(h1, geo.x, geo.y);
     print_number(h2, geo.x, geo.y + 7);

     attron(COLOR_PAIR(1));
     mvaddstr(geo.x + 1, geo.y + 15, "  ");
     mvaddstr(geo.x + 3, geo.y + 15, "  ");
     attroff(COLOR_PAIR(1));

     print_number(m1, geo.x, geo.y + 19);
     print_number(m2, geo.x, geo.y + 26);

     if(option.second)
     {
          attron(COLOR_PAIR(1));
          mvaddstr(geo.x + 1, geo.y + 34, "  ");
          mvaddstr(geo.x + 3, geo.y + 34, "  ");
          attroff(COLOR_PAIR(1));

          print_number(s1, geo.x, geo.y + 38);
          print_number(s2, geo.x, geo.y + 45);
     }

     /* Frame border */
     for(i = geo.y + DEPTHB; i < geo.y + geo.width; ++i)
     {
          mvaddch(geo.x + DEPTHB, i, ACS_HLINE);
          mvaddch(geo.x + geo.height, i, ACS_HLINE);
     }

     for (i = geo.x + DEPTHB; i < geo.x + geo.height; ++i)
     {
          mvaddch(i, geo.y + DEPTHB,    ACS_VLINE);
          mvaddch(i, geo.y + geo.width, ACS_VLINE);
     }

     /* Frame corner */
     mvaddch(geo.x + DEPTHB,     geo.y + DEPTHB,    ACS_ULCORNER);
     mvaddch(geo.x + geo.height, geo.y + DEPTHB,    ACS_LLCORNER);
     mvaddch(geo.x + DEPTHB,     geo.y + geo.width, ACS_URCORNER);
     mvaddch(geo.x + geo.height, geo.y + geo.width, ACS_LRCORNER);

     move(geo.x + geo.height + 1, geo.y + temp_dp);
     attron(COLOR_PAIR(3));
     printw("%d/%d/%d %s",
            sdate.month_day,
            sdate.month,
            sdate.year,
            meridiem);
     attroff(COLOR_PAIR(3));
}

/* KEY CHECKING FUNCTION */
void
check_key(Bool keylock)
{
     int c;

     if(!keylock)
          return;

     c = getch();

     switch(c)
     {
     case KEY_UP:
     case 'k':
     case 'K':
          if(!fixcenter)
          {
               if(geo.x > 1)
                    --geo.x;
               clear();
          }
          break;
     case KEY_DOWN:
     case 'j':
     case 'J':
          if(!fixcenter)
          {
               if(geo.x + geo.height + 2 < MAXH)
                    ++geo.x;
               clear();
          }
          break;
     case KEY_LEFT:
     case 'h':
     case 'H':
          if(!fixcenter)
          {
               if(geo.y > 1)
                    --geo.y;
               clear();
          }
          break;
     case KEY_RIGHT:
     case 'l':
     case 'L':
          if(!fixcenter)
          {
               if(geo.y + geo.width + 1 < MAXW)
                    ++geo.y;
               clear();
          }
          break;
     case 's':
     case 'S':
          if(!option.second)
               geo.width += DIFFSEC;
          else
               geo.width -= DIFFSEC;
          clear();
          option.second = !option.second;
          if(fixcenter)
          {
               fixcenter = False;
               set_center();
          }
          break;
     case 't':
     case 'T':
          clear();
          option.twelve = !option.twelve;
          break;
     case 'c':
     case 'C':
          clear();
          set_center();
          break;
     case 'q':
     case 'Q':
          endwin();
          exit(EXIT_SUCCESS);
          break;
     }
}

/* GETTING TIME FUNCTION */
void
get_time(void)
{
     int ihour;
     tm = localtime(&lt);
     lt = time(NULL);

     ihour = tm->tm_hour;

     if (option.twelve && ihour > 12)
          meridiem = "(PM)";
     else if (option.twelve && ihour < 12)
          meridiem = "(AM)";
     else
          meridiem = "  ";

     ihour = (option.twelve && ihour > 12) ? ihour - 12 : ihour;
     ihour = (option.twelve && !ihour) ? 12 : ihour;

     sdate.hour[0] = ihour / 10;
     sdate.hour[1] = ihour % 10;

     sdate.minute[0] = tm->tm_min / 10;
     sdate.minute[1] = tm->tm_min % 10;

     sdate.month_day = tm->tm_mday;
     sdate.month = tm->tm_mon + 1;
     sdate.year = tm->tm_year + 1900;

     if(option.second)
     {
          sdate.second[0] = tm->tm_sec / 10;
          sdate.second[1] = tm->tm_sec % 10;
     }
}

/* SET CENTER FUNCTION */
void
set_center(void)
{
     if(!fixcenter)
     {
          geo.y = MAXW / 2 - ((geo.width) / 2);
          geo.x = MAXH / 2 - (geo.height / 2);
          fixcenter = True;
     }
     else
          fixcenter = !fixcenter;
}

/* RUN FUCTION */
void
run(void)
{
     get_time();
     arrange_clock(sdate.hour[0], sdate.hour[1],
                   sdate.minute[0], sdate.minute[1],
                   sdate.second[0], sdate.second[1]);
     refresh();
     halfdelay(1);
}

/* MAIN FUCTION */

int
main(int argc, char **argv)
{
     int c;

     while ((c = getopt_long(argc,argv,"tx:y:vsbcih",
                             long_options, NULL)) != -1)
     {
          switch(c)
          {
          case 'h':
          default: printh(); exit(EXIT_SUCCESS); break;
          case 'i':
               printf("TTY-Clock ,Martin Duquesnoy© (xorg62@gmail.com)\n");
               exit(EXIT_SUCCESS);
               break;
          case 'v':
               printf("TTY-Clock v0.1.4\n");
               exit(EXIT_SUCCESS);
               break;
          case 'x':
               if(atoi(optarg)<0)
                    exit(EXIT_FAILURE);
               else
                    geo.x = atoi(optarg) + 1;

               break;
          case 'y':
               if(atoi(optarg)<0)
                    exit(EXIT_FAILURE);
               else
                    geo.y = atoi(optarg) + 1;
               break;
          case 's': option.second = True; break;
          case 't': option.twelve = True; break;
          case 'b': option.keylock = False; break;
          case 'c': start(); set_center(); break;
          }
     }

     start();
     run();

     for(;;)
     {
          usleep(10000);
          check_key(option.keylock);
          run();
     }

     endwin();

     return 0;
}
