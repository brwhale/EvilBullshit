class GameEntity {
public:
	int x, y;
	int w, h;
	int direction; //left=0,right,up,down
	vector<HBITMAP> Sprites;
	HWND window;
	int currentFrame;

	GameEntity(int _x, int _y, int _w, int _h, vector<HBITMAP> spritemap, HWND wind) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
		Sprites = spritemap;
		direction = 1;
		currentFrame = 0;
		window = wind;
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
	}
	void draw() {
		SetSplashImage(window, Sprites[direction * 2 + currentFrame/100], x, y, false);
	}
};