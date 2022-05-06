#ifndef __ESP_H__
#define __ESP_H__

typedef struct {
    void (*espWriteATCommand) (uint8_t *cmd, uint8_t nmbBytes);
}espConfig;

void espInit(espConfig *config);
void sendToESP(uint8_t *recv_buf, uint8_t recv_len);

#endif