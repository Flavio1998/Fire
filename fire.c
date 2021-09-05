/****************************************************************************
 *  Author: Flavio Peter <flaviop313@gmail.com> 2021                        *
 ****************************************************************************/

#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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

void draw(unsigned int fb[][LINES]) {
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

void spreadFire(unsigned int x, unsigned int y, unsigned int fb[][LINES]) { 
  // fire propagates randomly up, left and right 
  unsigned int ry = rand() % 4; // ry in [ 0 ; 3 ]
  unsigned int rx = rand() % 3; // rx in [ 0 ; 2 ]
  
  // With each iteration the heat gets cooler
  int heat = fb[x][y] - 1;
  if (heat < 100)
     heat = 0;
  
  int ny = y - ry;
  if (ny < 0) // no negativ index
    ny = 0;
  if (ny >= LINES-1) // bottom most line has to stay white.
    ny = LINES-2;

  int nx = x - rx + 1;
  if (nx < 0) // no negativ index
    nx = 0;
  if (nx >= COLS) // don't go out of array
    nx = COLS - 1;

  fb[nx][ny] = heat;
}

void doFire(unsigned int fb[][LINES]) {
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
}

int main(void)
{
  srand(time(NULL)); // init random-numbers
  initscr(); // has to be called here, so COLS and LINES are defined
  unsigned int framebuffer[COLS][LINES];
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
  clear();

  // loop to let the fire burn
  while (1) {
    doFire(framebuffer);
    draw(framebuffer);
    usleep(70000);
  }

  quit();
  return(0);
}
