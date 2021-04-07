#include <iostream>
#include <thread>
#include <chrono>

using namespace std;


const int window_width = 400;
const int window_height = 100;

int window[window_width][window_height];

const string block("\u2588");
const string colors[37] = {"\033[38;2;7;7;7m", 
                           "\033[38;2;31;7;7m", 
                           "\033[38;2;47;15;7m", 
                           "\033[38;2;71;15;7m", 
                           "\033[38;2;87;23;7m", 
                           "\033[38;2;103;31;7m", 
                           "\033[38;2;119;31;7m",
                           "\033[38;2;143;39;7m",
                           "\033[38;2;159;47;7m",
                           "\033[38;2;175;63;7m",
                           "\033[38;2;191;71;7m",
                           "\033[38;2;199;71;7m",
                           "\033[38;2;223;79;7m",
                           "\033[38;2;223;87;7m",
                           "\033[38;2;223;87;7m",
                           "\033[38;2;223;95;7m",
                           "\033[38;2;215;103;15m",
                           "\033[38;2;207;111;15m",
                           "\033[38;2;207;119;15m",
                           "\033[38;2;207;127;15m",
                           "\033[38;2;207;135;23m",
                           "\033[38;2;199;135;23m",
                           "\033[38;2;199;143;23m",
                           "\033[38;2;199;151;31m",
                           "\033[38;2;191;159;31m",
                           "\033[38;2;191;159;31m",
                           "\033[38;2;191;167;39m",
                           "\033[38;2;191;159;31m",
                           "\033[38;2;191;159;31m",
                           "\033[38;2;191;175;47m",
                           "\033[38;2;183;175;47m",
                           "\033[38;2;183;183;47m",
                           "\033[38;2;183;183;55m",
                           "\033[38;2;207;207;111m",
                           "\033[38;2;223;223;159m",
                           "\033[38;2;239;239;199m", 
                           "\033[38;2;255;255;255m"};

void displayWindow();
void spreadFire(int x, int y);
void doFire();

int main(void) {
    // Setup
    for (int y = 0; y <= window_height-1; ++y) {
        for (int x = 0; x <= window_width-1; ++x) {
            window[x][y] = 0;
        }
    }
    for (int x = 0; x <= window_width-1; ++x) {
        window[x][window_height-1] = 36;
    }



    for (int i = 0; i < 200; ++i) {
        system("clear");
        //cout << "\033[2J\033[1;1H";
        //cout << "\e[1;1H\e[2J";
        
        doFire();
        displayWindow();
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    return 0;
}

void displayWindow() {
    for (int y = 0; y <= window_height-1; ++y) {
        for (int x = 0; x <= window_width-1; ++x) {
            cout << colors[window[x][y]];
            cout << block;
        }
        cout << endl;
    }
}

void doFire() {
    for(int y = 0 ; y < window_height; ++y) {
        for (int x = 0; x < window_width; ++x) {
            spreadFire(x,y);
        }
    }
}

void spreadFire(int x, int y) {
    int ry = rand() % 4;
    int rx = rand() % 2;
    
    int y_new = y - ry;
    if (y_new < 0)
        y_new = 0;
    
    if (y_new >= window_height-1)
        y_new = window_height-2;


    int x_new;

    if (y_new > (double)window_height* 3.0/4.0) {
        if (rand() % 2 == 0)
            x_new = x - rx;
        else 
            x_new = x + rx;
    } else {
        x_new = x + rx;
    }
    if (x_new < 0) 
            x_new = 0;
        if (x_new >= window_width)
            x_new = window_width - 1;



    int color_new = window[x][y] - 1;
    if (color_new == -1)
        color_new = 0;
    
    window[x_new][y_new] = color_new;
}
