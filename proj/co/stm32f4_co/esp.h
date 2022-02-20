#ifndef __ESP_H__
#define __ESP_H__

void initESP(void);
void sendATcmd(char *cmd, uint8_t len);
void sendToESP(uint8_t *recv_buf, uint8_t recv_len);

#endif