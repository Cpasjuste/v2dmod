/*
typedef struct {
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
} BMFont_Char;

typedef struct {
    int size;
    int outline; // 1 for using outline, 0 otherwise
    char pagefile[256]; // where the font image sheet is
    BMFont_Char chr[95]; // there are 95 chars between ASCII 32 and 126
    int charset; // one of BMFont_CharsetEnum
    int unicode; // 1 for unicode characters, 0 otherwise
    int pages; // num of pages the characters are spread across
    int alphaChnl; // one of BMFont_ChnlHoldsEnum
    int redChnl; // one of BMFont_ChnlHoldsEnum
    int greenChnl; // one of BMFont_ChnlHoldsEnum
    int blueChnl; // one of BMFont_ChnlHoldsEnum
    int packed; // 1 if packed, 0 otherwise
} BMFont;

extern int BMFont_ParseText(const char *filename, BMFont *inf);

BMFont bmf = {25, 0, "",
              {
                      {120, 82, 1, 1, 0, 0, 4},
                      {122, 82, 1, 1, 0, 0, 4},
              }, 0, 0, 1, 0, 0, 0, 0, 0};
             */