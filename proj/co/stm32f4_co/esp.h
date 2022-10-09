#ifndef __ESP_H__
#define __ESP_H__

#include "main.h"

typedef struct {
    void (*espWriteATCommand) (uint8_t *cmd, uint8_t nmbBytes);
}espConfig;

void espInit(espConfig *config);
void espWrite(const CoParamType *p);

#endif