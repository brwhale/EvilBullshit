#include "stdlib.h"

#define D_LEFT   1
#define D_RIGHT 2
#define D_UP 4
#define D_DOWN 8
class GameEntity {
public:
	int x, y;
	int w, h;
	int direction; //left=0,right,up,down
	vector<HBITMAP> Sprites;
	HWND window;
	int currentFrame;
	int blockedDirections;
	int lockTimer;
	bool directionLocked;
	int speed = 5;
	GameEntity() {}
	GameEntity(int _x, int _y, int _w, int _h, HWND wind = 0, vector<HBITMAP> spritemap = {}) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
		Sprites = spritemap;
		direction = -1;
		currentFrame = 0;
		directionLocked = false;
		blockedDirections = 0;
		lockTimer = 10;
		if (wind == 0) {
			window = CreateSplashWindow(hInst);
		}
		else {
			window = wind;
		}
	}

	int hitTest(GameEntity* other) {
		int diffx1 = x - (other->x + other->w);
		int diffx2 = other->x - (x+w);
		int diffy1 = y - (other->y + other->h);
		int diffy2 = other->y - (y + h);
		int ret = 0;
		if (diffx1 < 0 && diffx2 < 0 && diffy1 < 0 && diffy2 < 0) {
			int lowestx = min(diffx1, diffx2);
			int lowesty = min(diffy1, diffy2);
			if (max(diffy1, diffy2) < -speed) {
				if (lowestx == diffx1) {
					ret |= D_RIGHT;
				}
				else if (lowestx == diffx2) {
					ret |= D_LEFT;
				}
			}
			if (max(diffx1, diffx2) < -speed) {
				if (lowesty == diffy1) {
					ret |= D_DOWN;
				}
				else if (lowesty == diffy2) {
					ret |= D_UP;
				}
			}
		}
		return ret;
	}

	int target(int posx, int posy) {
		auto xdiff = posx - x;
		auto ydiff = posy - y;
		auto axdiff = abs(xdiff);
		auto aydiff = abs(ydiff);
		if (directionLocked) return axdiff + aydiff;
		auto oldD = direction;
		if (axdiff > aydiff) {
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
		return axdiff + aydiff;
	}

	void LoadSpritemap(vector<int> spriteIds) {
		for (auto&& id : spriteIds) {
			Sprites.push_back(LoadSplashImage(id));
		}
	}

	void update() {
		currentFrame++;
		if (currentFrame >= 20) {
			currentFrame = 0;
		}

		switch (direction)
		{
		case 0: !(blockedDirections & D_LEFT) && (x-= speed); break;
		case 1: !(blockedDirections & D_RIGHT) && (x+=speed); break;
		case 2: !(blockedDirections & D_UP) && (y-= speed); break;
		case 3: !(blockedDirections & D_DOWN) && (y+= speed); break;
		default:
			return;
			break;
		}
		x = rotclamp(x, 0, maxx - w);
		y = rotclamp(y, 0, maxy - h);

		if (directionLocked) {
			if (--lockTimer < 0) {
				directionLocked = false;
				lockTimer = rand()%256;
			}
		}
	}
	void draw() {
		if (Sprites.size() > 1) {
			int dFix = direction;
			if (dFix < 0) dFix = 0;
			SetSplashImage(window, Sprites[dFix * 2 + currentFrame / 10], x, y, false);
		}
		else {
			SetSplashImage(window, Sprites[0], x, y, false);
		}
	}
};