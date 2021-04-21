/*
 *      TTY-CLOCK Main file.
 *      Copyright © 2009-2021 tty-clock contributors
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

#include "ttyclock.h"

void
init(void)
{
     struct sigaction sig;
     setlocale(LC_TIME,"");

     ttyclock.bg = COLOR_BLACK;

     /* Return codes:
      *    0 (EXIT_SUCCESS) = terminated nicely, either with 'q' or with timeout (-p option)
      *    1 (EXIT_FAILURE) = terminated abnormaly
      *    2= terminated with signal (eg. control-C)
      */
     ttyclock.retcode = EXIT_SUCCESS;

     /* Init ncurses */
     if (ttyclock.tty) {
          FILE *ftty = fopen(ttyclock.tty, "r+");
          if (!ftty) {
               fprintf(stderr, "tty-clock: error: '%s' couldn't be opened: %s.\n",
                       ttyclock.tty, strerror(errno));
               exit(EXIT_FAILURE);
          }
          ttyclock.ttyscr = newterm(NULL, ftty, ftty);
          assert(ttyclock.ttyscr != NULL);
          set_term(ttyclock.ttyscr);
     } else
          initscr();

     cbreak();
     noecho();
     keypad(stdscr, true);
     start_color();
     curs_set(false);
     clear();

     /* Init default terminal color */
     if(use_default_colors() == OK)
          ttyclock.bg = -1;

     /* Init color pair */
     init_pair(0, ttyclock.bg, ttyclock.bg);
     init_pair(1, ttyclock.bg, ttyclock.option.color);
     init_pair(2, ttyclock.option.color, ttyclock.bg);
//     init_pair(0, ttyclock.bg, ttyclock.bg);
//     init_pair(1, ttyclock.bg, ttyclock.option.color);
//     init_pair(2, ttyclock.option.color, ttyclock.bg);
     refresh();

     /* Init signal handler */
     sig.sa_handler = signal_handler;
     sig.sa_flags   = 0;
     sigaction(SIGTERM,  &sig, NULL);
     sigaction(SIGINT,   &sig, NULL);
     sigaction(SIGSEGV,  &sig, NULL);

     /* Init global struct */
     ttyclock.running = true;
     if(!ttyclock.geo.x)
          ttyclock.geo.x = 0;
     if(!ttyclock.geo.y)
          ttyclock.geo.y = 0;
     if(!ttyclock.geo.a)
          ttyclock.geo.a = 1;
     if(!ttyclock.geo.b)
          ttyclock.geo.b = 1;
     ttyclock.geo.w = compute_screen_width();
     ttyclock.geo.h = 7;
     ttyclock.tm = localtime(&(ttyclock.lt));
     if(ttyclock.option.utc) {
          ttyclock.tm = gmtime(&(ttyclock.lt));
     }
     ttyclock.lt = time(NULL);
     ttyclock.lt_origin = ttyclock.lt;
     
     update_hour();

     /* Create clock win */
     ttyclock.framewin = newwin(ttyclock.geo.h,
                                ttyclock.geo.w,
                                ttyclock.geo.x,
                                ttyclock.geo.y);
     if(ttyclock.option.box) {
          box(ttyclock.framewin, 0, 0);
     }

     if (ttyclock.option.bold)
     {
          wattron(ttyclock.framewin, A_BLINK);
     }

     /* Create the date win */
     ttyclock.datewin = newwin(DATEWINH, strlen(ttyclock.date.datestr) + 2,
                               ttyclock.geo.x + ttyclock.geo.h - 1,
                               ttyclock.geo.y + (ttyclock.geo.w / 2) -
                               (strlen(ttyclock.date.datestr) / 2) - 1);
     if(ttyclock.option.box && ttyclock.option.date) {
          box(ttyclock.datewin, 0, 0);
     }
     clearok(ttyclock.datewin, true);

     set_center(ttyclock.option.center);

     nodelay(stdscr, true);

     if (ttyclock.option.date)
     {
          wrefresh(ttyclock.datewin);
     }

     wrefresh(ttyclock.framewin);

     return;
}

void
signal_handler(int signal)
{
     switch(signal)
     {
     case SIGINT:
     case SIGTERM:
          ttyclock.retcode = EXIT_INTERRUPTED;
          ttyclock.running = false;
          break;
          /* Segmentation fault signal */
     case SIGSEGV:
          ttyclock.retcode = EXIT_FAILURE;
          endwin();
          fprintf(stderr, "Segmentation fault.\n");
          exit(EXIT_FAILURE);
          break;
     }

     return;
}

void
cleanup(void)
{
     if (ttyclock.ttyscr)
          delscreen(ttyclock.ttyscr);

     free(ttyclock.tty);
}

void
update_hour(void)
{
     int ihour;
     char tmpstr[128];
     time_t shown_time;

     ttyclock.lt = time(NULL);
     shown_time = (ttyclock.lt_paused ? ttyclock.lt_paused : ttyclock.lt);

     if(ttyclock.option.elapsed)
     {
          shown_time -= ttyclock.lt_origin;
          ttyclock.tm = gmtime(&(shown_time));
     }
     else if(ttyclock.option.countdown && ttyclock.option.timeout > 0)
     {
          shown_time -= ttyclock.lt_origin;
          shown_time = ttyclock.option.timeout - shown_time;
          ttyclock.tm = gmtime(&(shown_time));
     }
     else
     {
          ttyclock.tm = localtime(&(shown_time));
          if(ttyclock.option.utc)
               ttyclock.tm = gmtime(&(shown_time));
     }

     ihour = ttyclock.tm->tm_hour;

     if(ttyclock.option.twelve)
          ttyclock.meridiem = ((ihour >= 12) ? PMSIGN : AMSIGN);
     else
          ttyclock.meridiem = "\0";

     /* Manage hour for twelve mode */
     ihour = ((ttyclock.option.twelve && ihour > 12)  ? (ihour - 12) : ihour);
     ihour = ((ttyclock.option.twelve && !ihour) ? 12 : ihour);

     /* Set hour */
     ttyclock.date.hour[0] = ihour / 10;
     ttyclock.date.hour[1] = ihour % 10;

     /* Set minutes */
     ttyclock.date.minute[0] = ttyclock.tm->tm_min / 10;
     ttyclock.date.minute[1] = ttyclock.tm->tm_min % 10;

     /* Set date string */
     strftime(tmpstr,
              sizeof(tmpstr),
              ttyclock.option.format,
              ttyclock.tm);
     sprintf(ttyclock.date.datestr, "%s%s", tmpstr, ttyclock.meridiem);

     /* Set seconds */
     ttyclock.date.second[0] = ttyclock.tm->tm_sec / 10;
     ttyclock.date.second[1] = ttyclock.tm->tm_sec % 10;

     return;
}

bool
timed_out(void)
{
     if(ttyclock.lt_paused || !ttyclock.option.timeout)
          return false;
     return (ttyclock.lt >=  ttyclock.lt_origin + ttyclock.option.timeout);
}

int
compute_screen_width(void)
{
     int w = FULLFRAMEW;
     if(!ttyclock.option.second)
          w -= CLKFIELDW;
     if(!ttyclock.option.hour)
          w -= CLKFIELDW;
     return w;
}

void
update_clock_layout(void)
{
     int new_w;
     int y_adj;

     if(!ttyclock.option.date)
     {
          wbkgdset(ttyclock.datewin, COLOR_PAIR(0));
          werase(ttyclock.datewin);
          wrefresh(ttyclock.datewin);
     }
 
     new_w = compute_screen_width();
     for(y_adj = 0; (ttyclock.geo.y - y_adj) > (COLS - new_w - 1); ++y_adj);
     clock_move(ttyclock.geo.x, (ttyclock.geo.y - y_adj), new_w, ttyclock.geo.h);
     set_center(ttyclock.option.center);
     return;
}

void
draw_number(int n, int x, int y)
{
     int i, sy = y;

     for(i = 0; i < 30; ++i, ++sy)
     {
          if(sy == y + 6)
          {
               sy = y;
               ++x;
          }

          if (ttyclock.option.bold)
               wattron(ttyclock.framewin, A_BLINK);
          else
               wattroff(ttyclock.framewin, A_BLINK);

          wbkgdset(ttyclock.framewin, COLOR_PAIR(number[n][i/2]));
          mvwaddch(ttyclock.framewin, x, sy, ' ');
     }
     wrefresh(ttyclock.framewin);

     return;
}

void
draw_clock(void)
{
     int xpos = 1;
     chtype dotcolor = COLOR_PAIR(1);
     if (ttyclock.option.blink && time(NULL) % 2 == 0 && !ttyclock.lt_paused)
          dotcolor = COLOR_PAIR(2);

     /* Draw hour numbers */
     if (ttyclock.option.hour)
     {
          draw_number(ttyclock.date.hour[0], 1, xpos);
          xpos += 7;
          draw_number(ttyclock.date.hour[1], 1, xpos);
          xpos += 8;

          /* 2 dot for number separation */
          wbkgdset(ttyclock.framewin, dotcolor);
          mvwaddstr(ttyclock.framewin, 2, xpos, "  ");
          mvwaddstr(ttyclock.framewin, 4, xpos, "  ");
          xpos += 4;
     }

     /* Draw minute numbers */
     draw_number(ttyclock.date.minute[0], 1, xpos);
     xpos += 7;
     draw_number(ttyclock.date.minute[1], 1, xpos);
     xpos += 8;

     /* Draw second if the option is enabled */
     if (ttyclock.option.second)
     {
          /* Again 2 dot for number separation */
          wbkgdset(ttyclock.framewin, dotcolor);
          mvwaddstr(ttyclock.framewin, 2, xpos, "  ");
          mvwaddstr(ttyclock.framewin, 4, xpos, "  ");
          xpos += 4;

          /* Draw second numbers */
          draw_number(ttyclock.date.second[0], 1, xpos);
          xpos += 7;
          draw_number(ttyclock.date.second[1], 1, xpos);
          xpos += 8;
     }

     /* Draw the date */
     if (ttyclock.option.date)
     {
          if (ttyclock.option.bold)
               wattron(ttyclock.datewin, A_BOLD);
          else
               wattroff(ttyclock.datewin, A_BOLD);

          wbkgdset(ttyclock.datewin, (COLOR_PAIR(2)));
          mvwprintw(ttyclock.datewin, (DATEWINH / 2), 1, ttyclock.date.datestr);
          wrefresh(ttyclock.datewin);
     }

     return;
}

void
clock_move(int x, int y, int w, int h)
{

     /* Erase border for a clean move */
     wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
     wborder(ttyclock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
     werase(ttyclock.framewin);
     wrefresh(ttyclock.framewin);

     if (ttyclock.option.date)
     {
          wbkgdset(ttyclock.datewin, COLOR_PAIR(0));
          wborder(ttyclock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
          werase(ttyclock.datewin);
          wrefresh(ttyclock.datewin);
     }

     /* Frame win move */
     mvwin(ttyclock.framewin, (ttyclock.geo.x = x), (ttyclock.geo.y = y));
     wresize(ttyclock.framewin, (ttyclock.geo.h = h), (ttyclock.geo.w = w));

     /* Date win move */
     if (ttyclock.option.date)
     {
          mvwin(ttyclock.datewin,
                ttyclock.geo.x + ttyclock.geo.h - 1,
                ttyclock.geo.y + (ttyclock.geo.w / 2) - (strlen(ttyclock.date.datestr) / 2) - 1);
          wresize(ttyclock.datewin, DATEWINH, strlen(ttyclock.date.datestr) + 2);

          if (ttyclock.option.box) {
               box(ttyclock.datewin,  0, 0);
          }
     }

     if (ttyclock.option.box)
     {
          box(ttyclock.framewin, 0, 0);
     }

     wrefresh(ttyclock.framewin);
     wrefresh(ttyclock.datewin); 
     return;
}

/* Useless but fun :) */
void
clock_rebound(void)
{
     if(!ttyclock.option.rebound)
          return;

     if(ttyclock.geo.x < 1)
          ttyclock.geo.a = 1;
     if(ttyclock.geo.x > (LINES - ttyclock.geo.h - DATEWINH))
          ttyclock.geo.a = -1;
     if(ttyclock.geo.y < 1)
          ttyclock.geo.b = 1;
     if(ttyclock.geo.y > (COLS - ttyclock.geo.w - 1))
          ttyclock.geo.b = -1;

     clock_move(ttyclock.geo.x + ttyclock.geo.a,
                ttyclock.geo.y + ttyclock.geo.b,
                ttyclock.geo.w,
                ttyclock.geo.h);

     return;
}

void
set_center(bool b)
{
     if((ttyclock.option.center = b))
     {
          ttyclock.option.rebound = false;

          clock_move((LINES / 2 - (ttyclock.geo.h / 2)),
                     (COLS  / 2 - (ttyclock.geo.w / 2)),
                     ttyclock.geo.w,
                     ttyclock.geo.h);
     }

     return;
}

void
set_box(bool b)
{
     ttyclock.option.box = b;

     wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
     wbkgdset(ttyclock.datewin, COLOR_PAIR(0));

     if(ttyclock.option.box) {
          wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
          wbkgdset(ttyclock.datewin, COLOR_PAIR(0));
          box(ttyclock.framewin, 0, 0);
          if (ttyclock.option.date)
               box(ttyclock.datewin,  0, 0);
     }
     else {
          wborder(ttyclock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
          wborder(ttyclock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
     }

     wrefresh(ttyclock.datewin);
     wrefresh(ttyclock.framewin);
}

void
key_event(void)
{
     int i, c;

     struct timespec length = { ttyclock.option.delay, ttyclock.option.nsdelay };
     
     fd_set rfds;
     FD_ZERO(&rfds);
     FD_SET(STDIN_FILENO, &rfds);

     if (ttyclock.option.screensaver)
     {
          c = wgetch(stdscr);
          if(c != ERR && ttyclock.option.noquit == false)
          {
               ttyclock.running = false;
          }
          else
          {
               nanosleep(&length, NULL);
               for(i = 0; i < 8; ++i)
                    if(c == (i + '0'))
                    {
                         ttyclock.option.color = i;
                         init_pair(1, ttyclock.bg, i);
                         init_pair(2, i, ttyclock.bg);
                    }
          }
          return;
     }


     switch(c = wgetch(stdscr))
     {
     case KEY_RESIZE:
          endwin();
          init();
          break;

     case KEY_UP:
     case 'k':
     case 'K':
          if(ttyclock.geo.x >= 1
             && !ttyclock.option.center)
               clock_move(ttyclock.geo.x - 1, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
          break;

     case KEY_DOWN:
     case 'j':
     case 'J':
          if(ttyclock.geo.x <= (LINES - ttyclock.geo.h - DATEWINH)
             && !ttyclock.option.center)
               clock_move(ttyclock.geo.x + 1, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
          break;

     case KEY_LEFT:
     case 'h':
     case 'H':
          if(ttyclock.geo.y >= 1
             && !ttyclock.option.center)
               clock_move(ttyclock.geo.x, ttyclock.geo.y - 1, ttyclock.geo.w, ttyclock.geo.h);
          break;

     case KEY_RIGHT:
     case 'l':
     case 'L':
          if(ttyclock.geo.y <= (COLS - ttyclock.geo.w - 1)
             && !ttyclock.option.center)
               clock_move(ttyclock.geo.x, ttyclock.geo.y + 1, ttyclock.geo.w, ttyclock.geo.h);
          break;

     case 'q':
     case 'Q':
          if (ttyclock.option.noquit == false)
          {
               ttyclock.running = false;
               ttyclock.retcode = EXIT_SUCCESS;
          }
          break;

     case 's':
     case 'S':
          ttyclock.option.second = !ttyclock.option.second;
          update_clock_layout();
          break;

     case 'm':
     case 'M':
          /* Sad: legacy "move left" is H, so "hour" is M :( */
          ttyclock.option.hour = !ttyclock.option.hour;
          update_clock_layout();
          break;

     case 'd':
     case 'D':
          ttyclock.option.date = !ttyclock.option.date;
          update_clock_layout();
          break;

     case 'e':
     case 'E':
          ttyclock.option.elapsed = !ttyclock.option.elapsed;
          if(ttyclock.option.elapsed)
          {
               ttyclock.option.date = false;
               update_clock_layout();
          }
          break;

     case 't':
     case 'T':
          ttyclock.option.twelve = !ttyclock.option.twelve;
          /* Set the new ttyclock.date.datestr to resize date window */
          update_hour();
          clock_move(ttyclock.geo.x, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
          break;

     case 'c':
     case 'C':
          set_center(!ttyclock.option.center);
          break;

     case 'b':
     case 'B':
          ttyclock.option.bold = !ttyclock.option.bold;
          break;

     case 'r':
     case 'R':
          ttyclock.option.rebound = !ttyclock.option.rebound;
          if(ttyclock.option.rebound && ttyclock.option.center)
               ttyclock.option.center = false;
          break;

     case 'x':
     case 'X':
          set_box(!ttyclock.option.box);
          break;

     case '0': case '1': case '2': case '3':
     case '4': case '5': case '6': case '7':
          i = c - '0';
          ttyclock.option.color = i;
          init_pair(1, ttyclock.bg, i);
          init_pair(2, i, ttyclock.bg);
          break;

     case 'z':
     case 'Z':
          if(ttyclock.lt_origin)
               ttyclock.lt_origin = ttyclock.lt;
          if(ttyclock.lt_paused)
               ttyclock.lt_paused = ttyclock.lt;
          break;

     case '+':
          if(ttyclock.lt_origin)
               ttyclock.lt_origin += 60;
          break;

     case '-':
          if(ttyclock.lt_origin)
          {
               ttyclock.lt_origin -= 60;
               if(ttyclock.lt  >=  ttyclock.lt_origin + ttyclock.option.timeout)
                    ttyclock.lt_paused= false; /* make it break */
          }
          break;

     case ' ':
          if(!ttyclock.lt_paused)
               ttyclock.lt_paused = ttyclock.lt;
          else
          {
               if(ttyclock.lt_origin)
               {
                    /* shift origin by the pause duration */
                    ttyclock.lt_origin += ttyclock.lt - ttyclock.lt_paused;
                    ttyclock.lt_paused = 0;
               }
               ttyclock.lt_paused = 0;
          }
          break;


     default:
          pselect(1, &rfds, NULL, NULL, &length, NULL);
     }

     return;
}

int
main(int argc, char **argv)
{
     int c;

     /* Alloc ttyclock */
     memset(&ttyclock, 0, sizeof(ttyclock_t));

     ttyclock.option.date = true;
     ttyclock.option.hour = true;

     /* Default date format */
     strncpy(ttyclock.option.format, "%F", sizeof (ttyclock.option.format));
     /* Default color */
     ttyclock.option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */
     /* Default delay */
     ttyclock.option.delay = 1; /* 1FPS */
     ttyclock.option.nsdelay = 0; /* -0FPS */
     ttyclock.option.blink = false;

     atexit(cleanup);

     while ((c = getopt(argc, argv, "hvisHDtuf:crd:a:C:Bxbep:SnT:")) != -1)
     {
          /* "semantically ordered": please keep in sync with this switch, with the tty-clock.1 manpage and README */
          /* Helper to generate the README: egrep '^ .*[ []-[a-zA-Z].*\\n"' ttyclock.c | sed -e 's/^ *"//' -e 's/\\n".*$//' -e 's/^\s*printf("//' */
          switch(c)
          {
          /* general information */
          case 'h':
          default:
               printf("usage : tty-clock [-hvisHDtucrbxBeSn] [-f format] [-d delay] [-a nsdelay] [-C [0-7]] [-p duration] [-T tty]\n"
                      "    -h            Show this page                                                   \n"
                      "    -v            Show tty-clock version                                           \n"
                      "    -i            Show some info about tty-clock                                   \n"
                      "    -s            Show seconds                                                     \n"
                      "    -H            Hide hour                                                        \n"
                      "    -D            Hide date                                                        \n"
                      "    -t            Set the hour in 12h format                                       \n"
                      "    -u            Use UTC time                                                     \n"
                      "    -f format     Set the date format                                              \n"
                      "    -c            Set the clock at the center of the terminal                      \n"
                      "    -r            Do rebound the clock                                             \n"
                      "    -d delay      Set the delay between two redraws of the clock. Default 1s       \n"
                      "    -a nsdelay    Additional delay between two redraws in nanoseconds. Default 0ns \n"
                      "    -C [0-7]      Set the clock color                                              \n"
                      "    -B            Enable blinking colon                                            \n"
                      "    -x            Show box                                                         \n"
                      "    -b            Use bold colors                                                  \n"
                      "    -e            Display elapsed time, not current time                           \n"
                      "    -p duration   Set count down duration in seconds. Exit when done               \n"
                      "    -S            Screensaver mode                                                 \n"
                      "    -n            Don't quit on keypress                                           \n"
                      "    -T tty        Display the clock on the specified terminal                      \n");
               exit(EXIT_SUCCESS);
               break;
          case 'v':
               puts("TTY-Clock " VERSIONSTR);
               exit(EXIT_SUCCESS);
               break;
          case 'i':
               puts("TTY-Clock " VERSIONSTR " © by Martin Duquesnoy (xorg62@gmail.com), Grey (grey@greytheory.net)");
               exit(EXIT_SUCCESS);
               break;

          /* shown data */
          case 's':
               ttyclock.option.second = true;
               break;
          case 'H':
               ttyclock.option.hour = false;
               break;
          case 'D':
               ttyclock.option.date = false;
               break;
          case 't':
               ttyclock.option.twelve = true;
               break;

          /* data format */
          case 'u':
               ttyclock.option.utc = true;
               break;
          case 'f':
               strncpy(ttyclock.option.format, optarg, 100);
               break;

          /* data layout */
          case 'c':
               ttyclock.option.center = true;
               break;
          case 'r':
               ttyclock.option.rebound = true;
               break;
          case 'd':
               if(atol(optarg) >= 0 && atol(optarg) < 100)
                    ttyclock.option.delay = atol(optarg);
               break;
          case 'a':
               if(atol(optarg) >= 0 && atol(optarg) < 1000000000)
                    ttyclock.option.nsdelay = atol(optarg);
               break;
          case 'C':
               if(atoi(optarg) >= 0 && atoi(optarg) < 8)
                    ttyclock.option.color = atoi(optarg);
               break;
          case 'B':
               ttyclock.option.blink = true;
               break;
          case 'x':
               ttyclock.option.box = true;
               break;
          case 'b':
               ttyclock.option.bold = true;
               break;

          /* special modes */
          case 'e':
               ttyclock.option.elapsed = true;
               ttyclock.option.date = false;
               break;
          case 'p':
               if(atol(optarg)>0) {
                    ttyclock.option.countdown = true;
                    ttyclock.option.timeout = atol(optarg);
                    ttyclock.option.date = false;
               }
               break;

          /* screensaver */
          case 'S':
               ttyclock.option.screensaver = true;
               break;
          case 'n':
               ttyclock.option.noquit = true;
               break;
          case 'T': {
               struct stat sbuf;
               if (stat(optarg, &sbuf) == -1) {
                    fprintf(stderr, "tty-clock: error: couldn't stat '%s': %s.\n",
                              optarg, strerror(errno));
                    exit(EXIT_FAILURE);
               } else if (!S_ISCHR(sbuf.st_mode)) {
                    fprintf(stderr, "tty-clock: error: '%s' doesn't appear to be a character device.\n",
                              optarg);
                    exit(EXIT_FAILURE);
               } else {
                    free(ttyclock.tty);
                    ttyclock.tty = strdup(optarg);
               }}
               break;
          }
     }

     init();
     attron(A_BLINK);
     while(ttyclock.running)
     {
          clock_rebound();
          update_hour();
          if(timed_out())
          {
               ttyclock.retcode = EXIT_SUCCESS;
               break;
          }
          draw_clock();
          key_event();
     }

     endwin();

     exit(ttyclock.retcode);
}


// vim: expandtab tabstop=5 softtabstop=5 shiftwidth=5
