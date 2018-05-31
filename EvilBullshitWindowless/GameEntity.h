#include "stdlib.h"
class GameEntity {
public:
	int x, y;
	int w, h;
	int direction; //left=0,right,up,down
	vector<HBITMAP> Sprites;
	HWND window;
	int currentFrame;
	int lockTimer;
	bool directionLocked;
	GameEntity() {}
	GameEntity(int _x, int _y, int _w, int _h, HWND wind = 0, vector<HBITMAP> spritemap = {}) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
		Sprites = spritemap;
		direction = 1;
		currentFrame = 0;
		directionLocked = false;
		lockTimer = 100;
		if (wind == 0) {
			window = CreateSplashWindow(hInst);
		}
		else {
			window = wind;
		}
	}

	void target(int posx, int posy) {
		if (directionLocked) return;
		auto xdiff = posx - x;
		auto ydiff = posy - y;
		auto oldD = direction;
		if (abs(xdiff) > abs(ydiff)) {
			if (xdiff > 0) {
				direction = 1;
			}
			else {
				direction = 0;
			}
		}
		else {
			if (ydiff > 0) {
				direction = 3;
			}
			else {
				direction = 2;
			}
		}
		if (oldD != direction) {
			// don't reverse straight up
			if (direction == 2 && oldD == 3) {
				direction -= (rand() % 2) + 1;
			} else if(direction == 3 && oldD == 2) {
				direction -= (rand() % 2) + 2;
			} else if (direction == 0 && oldD == 1) {
				direction += (rand() % 2) + 2;
			}
			else if (direction == 1 && oldD == 0) {
				direction += (rand() % 2) + 1;
			}
			directionLocked = true;
		}
	}

	void LoadSpritemap(vector<int> spriteIds) {
		for (auto&& id : spriteIds) {
			Sprites.push_back(LoadSplashImage(id));
		}
	}

	void update() {
		switch (direction)
		{
		case 0: x--; break;
		case 1: x++; break;
		case 2: y--; break;
		case 3: y++; break;
		default:
			break;
		}
		x = rotclamp(x, 0, maxx - w);
		y = rotclamp(y, 0, maxy - h);
		currentFrame++;
		if (currentFrame >= 200) {
			currentFrame = 0;
		}
		if (directionLocked) {
			if (--lockTimer < 0) {
				directionLocked = false;
				lockTimer = rand()%256;
			}
		}
	}
	void draw() {
		SetSplashImage(window, Sprites[direction * 2 + currentFrame/100], x, y, false);
	}
};