//
// Created by cpasjuste on 11/05/17.
//

#ifndef BMF2C_BMFONT_H
#define BMF2C_BMFONT_H

#include <vector>

class BMFont {

public:

    class BMChar {

    public:
        int x;
        int y;
        int width;
        int height;
        int xoffset;
        int yoffset;
        int xadvance;
    };

    int size;
    int chars_count;
    std::vector<BMChar> chars;  // there are 95 chars between ASCII 32 and 126
};


#endif //BMF2C_BMFONT_H
