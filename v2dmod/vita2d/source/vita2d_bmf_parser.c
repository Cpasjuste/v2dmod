#include <libk/stdio.h>
#include <libk/string.h>
#include <libk/stdlib.h>
#include <psp2/types.h>
#include <psp2/io/fcntl.h>

// ?! ?!
#include <psp2/kernel/threadmgr.h>
#define printf(...) sceKernelDelayThread(10)

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

#define BMFONT_TOKEN_LEX_LEN 256

#define streql(X, Y)  ( strcmp( X, Y ) == 0 )

#define BMFONT_SUCCESS 0
#define BMFONT_ERROR -1

enum BMFont_TokenTypeEnum {
    TOKENTYPE_UNKNOWN,
    TOKENTYPE_ID,
    TOKENTYPE_NUM,
    TOKENTYPE_STRING,
    TOKENTYPE_EQ,
    TOKENTYPE_COMMA
};

enum BMFont_DFAStatesEnum {
    STATE_START = 0,
    STATE_NUM = 1,
    STATE_OPEN_STR_EMPTY = 2,
    STATE_OPEN_STR = 3,
    STATE_STR = 4,
    STATE_EQ = 5,
    STATE_ID = 6,
    STATE_COMMA = 7
};

typedef struct BMFont_Token {
    int type;
    char lexeme[BMFONT_TOKEN_LEX_LEN];
} BMFont_Token;

enum BMFont_CharsetEnum {
    CHARSET_UNKNOWN, // for if charset isn't recognized as one of above
    CHARSET_NONE, // for unicode fonts (not supported)
    CHARSET_ANSI, // this is the only one we support
    CHARSET_ARABIC,
    CHARSET_BALTIC,
    CHARSET_EASTEUROPE,
    CHARSET_GREEK,
    CHARSET_HEBREW,
    CHARSET_RUSSIAN,
    CHARSET_TURKISH,
    CHARSET_VIETNAMESE
};

static int BMFont_TokenizeLine(char *line, int len, BMFont_Token *buffer);
static int BMFont_DoTransition(int state, char c);
static int BMFont_StateToType(int state);
static int BMFont_IsAcceptingState(int state);
static int BMFont_ParseTextInfo(BMFont_Token *tok, int numtoks, BMFont *inf);
static int BMFont_ParseTextCommon(BMFont_Token *tok, int numtoks, BMFont *inf);
static int BMFont_ParseTextPage(BMFont_Token *tok, int numtoks, BMFont *inf);
static int BMFont_ParseTextChar(BMFont_Token *tok, int numtoks, BMFont *inf);

int BMFont_CharsetToEnum(const char *charset) {
    if (*charset == '\0') return CHARSET_NONE;
    if (streql(charset, "ANSI")) return CHARSET_ANSI;
    if (streql(charset, "ARABIC")) return CHARSET_ARABIC;
    if (streql(charset, "BALTIC")) return CHARSET_BALTIC;
    if (streql(charset, "EASTEUROPE")) return CHARSET_EASTEUROPE;
    if (streql(charset, "GREEK")) return CHARSET_GREEK;
    if (streql(charset, "HEBREW")) return CHARSET_HEBREW;
    if (streql(charset, "RUSSIAN")) return CHARSET_RUSSIAN;
    if (streql(charset, "TURKISH")) return CHARSET_TURKISH;
    if (streql(charset, "VIETNAMESE")) return CHARSET_VIETNAMESE;

    return CHARSET_UNKNOWN;
}

static int read_line_file(SceUID fp, char *line, int num) {
    char buff[num];
    char *end;
    int len;
    int tmp;

    tmp = 0;
    len = sceIoRead(fp, buff, num);
    if (len == 0)
        return -1;

    end = strchr(buff, '\n');

    if (end == NULL) {
        buff[num - 1] = '\0';
        strcpy(line, buff);
        return len;
    }

    end[0] = '\0';
    if ((end != buff) && (end[-1] == '\r')) {
        end[-1] = '\0';
        tmp = -1;
    }

    strcpy(line, buff);
    sceIoLseek(fp, -len + (end - buff) + 1, SEEK_CUR);
    return end - buff + tmp;
}

int BMFont_ParseText(const char *filename, BMFont *inf) {

    SceUID fd;

    char line[512];
    int success = 0;

    if (inf == NULL) {
        printf("BMFont_ParseText: Given NULL ParseInfo struct!");
        return -1;
    }

    fd = sceIoOpen(filename, SCE_O_RDONLY, 6);
    if (fd < 0) {
        printf("BMFont_ParseText: Couldn't open file: %s", filename);
        return -1;
    }


    while (1) {

        BMFont_Token tok[256]; // 256 just to be safe
        int read = 0;
        int toknum = 0;
        int prind = 0;

        // read a line
        read = read_line_file(fd, line, 512);
        if (read <= 0) {
            break;
        }

        // tokenize the line
        toknum = BMFont_TokenizeLine(line, read, tok);
        if (toknum == -1) {
            // we make sure to set the error in BMFont_TokenizeLine
            // return -1;
            continue;
        }

        // DEBUG print the tokens
        for (prind = 0; prind < toknum; prind++) {
            //printf("TOKEN: %d (%s)\n", tok[prind].type, tok[prind].lexeme);
        }

        if (streql(tok[0].lexeme, "info")) {
            success = BMFont_ParseTextInfo(tok, toknum, inf);
        } else if (streql(tok[0].lexeme, "common")) {
            success = BMFont_ParseTextCommon(tok, toknum, inf);
        } else if (streql(tok[0].lexeme, "page")) {
            success = BMFont_ParseTextPage(tok, toknum, inf);
        } else if (streql(tok[0].lexeme, "char")) {
            success = BMFont_ParseTextChar(tok, toknum, inf);
        } else {
            printf("Unknown line type: %s", tok[0].lexeme);
        }

        if (success == BMFONT_ERROR) break;
    }

    sceIoClose(fd);

    return success;
}

static int BMFont_TokenizeLine(char *line, int len, BMFont_Token *buffer) {
    char lexbuf[BMFONT_TOKEN_LEX_LEN] = {'\0'};
    int laststate = STATE_START;
    int state = STATE_START;
    int indLine = 0;    // index within `line`
    int indBuffer = 0;  // index within `buffer'
    int indLex = 0;     // index within `lexbuf`

    while (indLine < len) {
        char c = line[indLine];

        // if we encounter end of line then we are done
        if (c == '\r' || c == '\n') break;

        // do transition, act on result
        laststate = state;
        state = BMFont_DoTransition(state, c);

        if (state == -1) {
            // transition failed

            if (laststate == STATE_START) {
                if (c == ' ') {
                    // ignore whitespace when on start state
                    indLine++;
                    laststate = state = STATE_START;
                    continue;
                }

                // ERROR can't do anything with character
                printf("Found unexpected char ASCII: %d", c);
                return BMFONT_ERROR;
            }

            // ERROR we're stuck at a non-accepting state!
            if (!BMFont_IsAcceptingState(laststate)) {
                printf("Error parsing at: %s (%d)", lexbuf, laststate);
                return BMFONT_ERROR;
            }

            // "create" token
            buffer[indBuffer].type = BMFont_StateToType(laststate);

            // copy lexeme buffer and reset
            memcpy(buffer[indBuffer].lexeme, lexbuf, indLex + 1);
            indLex = 0;
            lexbuf[indLex] = '\0';

            // reset state
            laststate = state = STATE_START;

            // increment token buffer index
            indBuffer++;

            // printf( "EMITTED TOKEN!\n" ); // DEBUG!
        } else {
            // transition succeeded

            // ERROR no room in lexeme buffer!
            if (indLex == BMFONT_TOKEN_LEX_LEN - 1) {
                // printf( "%s\n", lexbuf ); // DEBUG!
                printf("Exceeded token lexeme length of %d!",
                       BMFONT_TOKEN_LEX_LEN);
                return BMFONT_ERROR;
            }

            // add character to lexeme buffer, move to next (ignore " from str)
            if (state != STATE_OPEN_STR_EMPTY && state != STATE_STR) {
                lexbuf[indLex] = c;
                indLex++;
                lexbuf[indLex] = '\0';
            }

            indLine++;
        }
    }

    // do something with the remaining lexeme buffer

    // ERROR we're stuck at a non-accepting state!
    if (!BMFont_IsAcceptingState(state)) {
        printf("Error parsing at: %s", lexbuf);
        return BMFONT_ERROR;
    }

    // "create" the final token
    buffer[indBuffer].type = BMFont_StateToType(state);
    memcpy(buffer[indBuffer].lexeme, lexbuf, indLex + 1);
    indBuffer++;

    return indBuffer;
}


static int BMFont_DoTransition(int state, char c) {
    switch (state) {
        case STATE_START:
            if (c >= '0' && c <= '9') return STATE_NUM;
            if (c >= 'a' && c <= 'z') return STATE_ID;
            if (c >= 'A' && c <= 'Z') return STATE_ID;
            if (c == '"') return STATE_OPEN_STR_EMPTY;
            if (c == '=') return STATE_EQ;
            if (c == ',') return STATE_COMMA;
            if (c == '-') return STATE_NUM;
            break;
        case STATE_ID:
            if (c >= 'a' && c <= 'z') return STATE_ID;
            if (c >= 'A' && c <= 'Z') return STATE_ID;
            break;
        case STATE_NUM:
            if (c >= '0' && c <= '9') return STATE_NUM;
            break;
        case STATE_OPEN_STR:
            if (c == '"') return STATE_STR;
            return STATE_OPEN_STR;
        case STATE_OPEN_STR_EMPTY:
            if (c == '"') return STATE_STR;
            return STATE_OPEN_STR;
        case STATE_EQ:
        case STATE_STR:
        case STATE_COMMA:
            break;
    }

    // if we didn't match anything above then we failed
    return -1;
}


static int BMFont_StateToType(int state) {
    switch (state) {
        case STATE_ID:
            return TOKENTYPE_ID;
        case STATE_NUM:
            return TOKENTYPE_NUM;
        case STATE_EQ:
            return TOKENTYPE_EQ;
        case STATE_STR:
            return TOKENTYPE_STRING;
        case STATE_COMMA:
            return TOKENTYPE_COMMA;
    }

    // all other states are not accepting and have no equivalent
    return -1;
}


static int BMFont_IsAcceptingState(int state) {
    switch (state) {
        case STATE_NUM:
        case STATE_STR:
        case STATE_EQ:
        case STATE_ID:
        case STATE_COMMA:
            return 1;
    }

    // only the above states are accepting, otherwise not
    return 0;
}


static int BMFont_ParseTextInfo(BMFont_Token *tok, int numtoks, BMFont *inf) {
    int i = 1;

    while (i < numtoks) {
        if (tok[i].type == TOKENTYPE_ID) {
            char *name = tok[i].lexeme;
            char *content = NULL;

            // ERROR not enough tokens
            if (i + 1 >= numtoks || i + 2 >= numtoks) {
                printf("Parsing error: tag without value: %s", name);
                return BMFONT_ERROR;
            }

            // ERROR missing = sign between key and value
            if (tok[i + 1].type != TOKENTYPE_EQ) {
                printf("Parsing error: expected `=`, got `%s`",
                       tok[i + 1].lexeme);
                return BMFONT_ERROR;
            }

            content = tok[i + 2].lexeme;

            if (streql(name, "charset")) {
                inf->charset = BMFont_CharsetToEnum(content);
            } else if (streql(name, "unicode")) {
                inf->unicode = atoi(content);
            } else if (streql(name, "outline")) {
                inf->outline = atoi(content);
            } else if (streql(name, "size")) {
                inf->size = atoi(content);
            } else {
                printf("Unknown info tag: %s\n", name); // DEBUG
            }

            i += 3;
        } else {
            i++;
        }
    }

    return BMFONT_SUCCESS;
}


static int BMFont_ParseTextCommon(BMFont_Token *tok, int numtoks, BMFont *inf) {
    int i = 1;

    while (i < numtoks) {
        if (tok[i].type == TOKENTYPE_ID) {
            char *name = tok[i].lexeme;
            char *content = NULL;

            // ERROR not enough tokens
            if (i + 1 >= numtoks || i + 2 >= numtoks) {
                printf("Parsing error: tag without value: %s", name);
                return BMFONT_ERROR;
            }

            // ERROR missing = sign between key and value
            if (tok[i + 1].type != TOKENTYPE_EQ) {
                printf("Parsing error: expected `=`, got `%s`",
                       tok[i + 1].lexeme);
                return BMFONT_ERROR;
            }

            content = tok[i + 2].lexeme;

            if (streql(name, "pages")) {
                inf->pages = atoi(content);
            } else if (streql(name, "alphaChnl")) {
                inf->alphaChnl = atoi(content);
            } else if (streql(name, "redChnl")) {
                inf->redChnl = atoi(content);
            } else if (streql(name, "greenChnl")) {
                inf->greenChnl = atoi(content);
            } else if (streql(name, "blueChnl")) {
                inf->blueChnl = atoi(content);
            } else if (streql(name, "packed")) {
                inf->packed = atoi(content);
            } else {
                printf("Unknown common tag: %s\n", name); // DEBUG
            }

            i += 3;
        } else {
            i++;
        }
    }

    return BMFONT_SUCCESS;
}


static int BMFont_ParseTextPage(BMFont_Token *tok, int numtoks, BMFont *inf) {
    int i = 1;

    while (i < numtoks) {
        if (tok[i].type == TOKENTYPE_ID) {
            char *name = tok[i].lexeme;
            char *content = NULL;

            // ERROR not enough tokens
            if (i + 1 >= numtoks || i + 2 >= numtoks) {
                printf("Parsing error: tag without value: %s", name);
                return BMFONT_ERROR;
            }

            // ERROR missing = sign between key and value
            if (tok[i + 1].type != TOKENTYPE_EQ) {
                printf("Parsing error: expected `=`, got `%s`",
                       tok[i + 1].lexeme);
                return BMFONT_ERROR;
            }

            content = tok[i + 2].lexeme;

            if (streql(name, "id")) {
                int id = atoi(content);
                if (id != 0) break; // HACK only load page 0
            } else if (streql(name, "file")) {
                strncpy(inf->pagefile, content, 256);
                return 0; // HACK we found a file good enough
            } else {
                printf("Unknown page tag: %s\n", name); // DEBUG
            }

            i += 3;
        } else {
            i++;
        }
    }

    return BMFONT_SUCCESS;
}


static int BMFont_ParseTextChar(BMFont_Token *tok, int numtoks, BMFont *inf) {
    int i = 1;
    int id = 0;

    while (i < numtoks) {
        if (tok[i].type == TOKENTYPE_ID) {
            char *name = tok[i].lexeme;
            char *content = NULL;

            // ERROR not enough tokens
            if (i + 1 >= numtoks || i + 2 >= numtoks) {
                printf("Parsing error: tag without value: %s", name);
                return BMFONT_ERROR;
            }

            // ERROR missing = sign between key and value
            if (tok[i + 1].type != TOKENTYPE_EQ) {
                printf("Parsing error: expected `=`, got `%s`",
                       tok[i + 1].lexeme);
                return BMFONT_ERROR;
            }

            content = tok[i + 2].lexeme;

            if (streql(name, "id")) {
                id = atoi(content);
                if (id < 32 || id > 126) break; // ignore chars with bad id
                id -= 32;
            } else if (streql(name, "x")) {
                inf->chr[id].x = atoi(content);
            } else if (streql(name, "y")) {
                inf->chr[id].y = atoi(content);
            } else if (streql(name, "width")) {
                inf->chr[id].width = atoi(content);
            } else if (streql(name, "height")) {
                inf->chr[id].height = atoi(content);
            } else if (streql(name, "xoffset")) {
                inf->chr[id].xoffset = atoi(content);
            } else if (streql(name, "yoffset")) {
                inf->chr[id].yoffset = atoi(content);
            } else if (streql(name, "xadvance")) {
                inf->chr[id].xadvance = atoi(content);
            } else {
                printf("Unknown char tag: %s\n", name); // DEBUG
            }

            i += 3;
        } else {
            i++;
        }
    }

    return BMFONT_SUCCESS;
}
