typedef struct {
	int x;
	int y;
	int width;
	int height;
	int xoffset;
	int yoffset;
	int xadvance;
} BMChar;

typedef struct {
	int size;
	BMChar chars[95];
} BMFont;
