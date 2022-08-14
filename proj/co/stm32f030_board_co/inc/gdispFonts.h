#ifndef __GDISPFONTS_H__
#define __GDISPFONTS_H__

#include <stdint.h>

extern const char Times_New_Roman11x12[];
extern const char Times_New_Roman23x22[];
extern const char Times_New_Roman57x60[];
extern const char Times_New_Roman85x64[];

typedef enum {
    Font_Times_New_Roman11x12,
    Font_Times_New_Roman23x22,
    Font_Times_New_Roman57x60,
    Font_Times_New_Roman85x64,
}FontType;

typedef struct {

    const char *font_data;
    const uint8_t fontWidth;
    const uint8_t fontHeight;
    const uint16_t fontOneCharSize;
}FontCtx;

typedef struct {

    const FontCtx *fontCtx;
    FontType type;
    const uint8_t fontNmb;
}Font;

typedef enum {
    FONT_COMPLETED,
    FONT_NEXT_ROW,
    FONT_NEXT_COLUMN,
    FONT_UNKNOWN,
}FontStatus;

FontStatus gdispFontsGetFontByte(char sign, uint16_t *fontData);
uint8_t gdispFontGetSignSpan(char sign);
void gdispFontSetFontType(FontType type);

#endif