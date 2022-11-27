#include "menu.h"
#include "gdisp.h"

typedef struct{
    void(*action)(void);
    uint8_t *icon;
    uint8_t icon_size;
    uint8_t Xpos;
    uint8_t Ypos;
    uint8_t Xsize;
    uint8_t Ysize;
}menuItem;

typedef struct{
    menuItem *items;
    uint8_t itemsCnt;
}menuScreen;

void menuDraw(menuScreen *screen)
{
    for(uint8_t item = 0u; item < screen->itemsCnt; ++item)
    {
        gdispDrawItem(screen->items->icon, screen->items->Ypos, \
            screen->items->Xpos, screen->items->Xsize, screen->items->Ysize);
    }
}