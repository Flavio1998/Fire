/****************************************************************************
 *  Author: Flavio Peter <flaviop313@gmail.com> 09 2021                     *
 ****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <curses.h>
#include <sys/ioctl.h>

// Indicates a caught signal
volatile sig_atomic_t signal_status = 0; 

void sighandler(int s) {
    signal_status = s;
}

void usage() {
  printf(" Usage: fire -[h] [-i iterations] [-s speed]\n");
  printf(" -h: Print usage and exit\n");
  printf(" -i: Number of iterations before the fire goes out (-1 to keep the fire burning indefinitely)\n");
  printf(" -s: Speed to update the fire animation in milliseconds\n");
}

void freeHeap(unsigned int** fb) {
  // free memory used by framebuffer from heap
  for (int i = 0; i < COLS; ++i) {
    free(fb[i]);
  }
  free(fb);
}

void resizeScreen() {
  // set new terminal size
  struct winsize win;
  ioctl(0, TIOCGWINSZ, &win);
  COLS = win.ws_col;
  LINES = win.ws_row;
  resizeterm(LINES, COLS);

  endwin();
  refresh();
}

void initCustomColors() {
  unsigned int colors[37][3] = {
        {0x07, 0x07, 0x07},
        {0x1f, 0x07, 0x07},
        {0x2f, 0x0f, 0x07},
        {0x47, 0x0f, 0x07},
        {0x57, 0x17, 0x07},
        {0x67, 0x1f, 0x07},
        {0x77, 0x1f, 0x07},
        {0x8f, 0x27, 0x07},
        {0x9f, 0x2f, 0x07},
        {0xaf, 0x3f, 0x07},
        {0xbf, 0x47, 0x07},
        {0xc7, 0x47, 0x07},
        {0xdf, 0x4f, 0x07},
        {0xdf, 0x57, 0x07},
        {0xdf, 0x57, 0x07},
        {0xdf, 0x5f, 0x07},
        {0xd7, 0x67, 0x0f},
        {0xcf, 0x6f, 0x0f},
        {0xcf, 0x77, 0x0f},
        {0xcf, 0x7f, 0x0f},
        {0xcf, 0x87, 0x17},
        {0xc7, 0x87, 0x17},
        {0xc7, 0x8f, 0x17},
        {0xc7, 0x97, 0x1f},
        {0xbf, 0x9f, 0x1f},
        {0xbf, 0x9f, 0x1f},
        {0xbf, 0xa7, 0x27},
        {0xbf, 0x9f, 0x1f},
        {0xbf, 0x9f, 0x1f},
        {0xbf, 0xaf, 0x2f},
        {0xb7, 0xaf, 0x2f},
        {0xb7, 0xb7, 0x2f},
        {0xb7, 0xb7, 0x37},
        {0xcf, 0xcf, 0x6f},
        {0xdf, 0xdf, 0x9f},
        {0xef, 0xef, 0xc7},
        {0xff, 0xff, 0xff}
    };
  double scale = 1000 / 255;
  for (unsigned int i = 0; i < sizeof(colors) / sizeof(colors[0][0]); i += 3) {
    // mapping [ 0 ; 255 ] to [ 0 ; 1000 ], bacause curses wants r,g,b in a range from [ 0 ; 1000 ]
    // start colors and colorpairs from index 100 upwards, to not mess with default colors
    init_color(i/3 + 100, (int)(colors[i/3][0]*scale), (int)((colors[i/3][1])*scale), (int)((colors[i/3][2])*scale));
    init_pair(i/3 + i%3 + 100, i/3 + i%3 + 100, i/3 + i%3 + 100);
  }
  // colorpair 0 is black
  init_pair(0, COLOR_BLACK, COLOR_BLACK);
}

void draw(unsigned int** fb) {
  // dont' draw the last line, it's only white anyway
  for (unsigned int y = 0; y < LINES-1; ++y) {
    for (unsigned int x = 0; x < COLS; ++x) {
      // read and set colorpair from frambuffer array
      attrset(COLOR_PAIR(fb[x][y]));
      mvprintw(y, x, " ");
    }
  }
  refresh();
}

void spreadFire(unsigned int x, unsigned int y, unsigned int** fb) { 
  // fire propagates randomly up, left and right 
  unsigned int ry = rand() % 4; // ry in [ 0 ; 3 ]
  unsigned int rx = rand() % 3; // rx in [ 0 ; 2 ]
  
  // With each iteration the heat gets cooler
  int heat = fb[x][y] - 1;
  if (heat < 100)
     heat = 0;
  
  int ny = y - ry;
  // no negativ index
  if (ny < 0) 
    ny = 0;
  // bottom most line has to stay white
  if (ny >= LINES-1) 
    ny = LINES-2;

  int nx = x - rx + 1;
  // no negativ index
  if (nx < 0) 
    nx = 0;
  // don't go out of array
  if (nx >= COLS) 
    nx = COLS - 1;

  fb[nx][ny] = heat;
}

void doFire(unsigned int** fb) {
  // iterrate through the framebuffer and spread the fire
  for(unsigned int y = 0 ; y < LINES; ++y) {
    for (unsigned int x = 0; x < COLS; ++x) {
      spreadFire(x, y, fb);
    }
  }
}

void quit()
{
  endwin();
  exit(0);
}



int main(int argc, char* argv[])
{
  int optchr;
  unsigned int count = 0;
  int iterations = -1;
  // update cycle in ms
  int speed = 70;

  opterr = 0;
  while ((optchr = getopt(argc, argv, "hi:s:")) != EOF) {
    switch (optchr) {
      case 'h':
      case '?':
        usage();
        quit();
      case 'i': 
        iterations = atoi(optarg);
        break;
      case 's': 
        speed = atoi(optarg);
        break;
    }
  }

  // init random-numbers
  srand(time(NULL));
  // has to be called here, so COLS and LINES are defined
  initscr();
  // create framebuffer array using pionters, so it can be resized
  unsigned int** framebuffer = (unsigned int**)malloc(COLS * sizeof(unsigned int*));
  for (int i = 0; i < COLS; ++i)
    framebuffer[i] = (unsigned int*)malloc(LINES * sizeof(unsigned int));
  
  for (unsigned int y = 0; y < LINES; ++y) {
    for (unsigned int x = 0; x < COLS; ++x) {
      // fill framebuffer with black / colorpair 0
      framebuffer[x][y] = 0;
    }
  }
  for (unsigned int x = 0; x < COLS; ++x) {
    // bottom most line is the source of the fire, white color / colorpair: 136
    framebuffer[x][LINES-1] = 136;
  }
  atexit(quit);
  curs_set(0); // no blinking cursor
  start_color();
  initCustomColors();
  keypad(stdscr, true);
  clear();

  signal(SIGINT, sighandler);
  signal(SIGQUIT, sighandler);
  signal(SIGWINCH, sighandler);
  signal(SIGTSTP, sighandler);

  // loop to let the fire burn
  while (1) {

    if (signal_status == SIGINT || signal_status == SIGQUIT || signal_status == SIGTSTP) {
      freeHeap(framebuffer);
      quit();
    }

    if (signal_status == SIGWINCH) {
      freeHeap(framebuffer);
      resizeScreen();
      // reallocate memory for framebuffer with updated COLS and LINES
      framebuffer = (unsigned int**)malloc(COLS * sizeof(unsigned int*));
      for (int i = 0; i < COLS; ++i)
        framebuffer[i] = (unsigned int*)malloc(LINES * sizeof(unsigned int));
      for (unsigned int y = 0; y < LINES; ++y) {
        for (unsigned int x = 0; x < COLS; ++x) {
          // fill framebuffer with black / colorpair 0
          framebuffer[x][y] = 0;
        }
      }
      for (unsigned int x = 0; x < COLS; ++x) {
        // bottom most line is the source of the fire, white color / colorpair: 136
        framebuffer[x][LINES-1] = 136;
      }
      signal_status = 0;
    }

    if (iterations != -1 && count >= iterations) {
      if (count == iterations + 120)
        quit();
      for (unsigned int x = 0; x < COLS; ++x) {
        // set the bottom most line to 0, to extinguish the fire
        framebuffer[x][LINES-1] = 0;
      }
    }

    doFire(framebuffer);
    draw(framebuffer);
    usleep(speed * 1000);
    ++count;
  }

  freeHeap(framebuffer);
  quit();
  return(0);
}
