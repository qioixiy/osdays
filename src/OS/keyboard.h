#ifndef _KEYBOARD_H
#define _KEYBOARD_H

//º¸≈Ã Û±Í…Ë÷√
#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0X0064
#define PORT_KEYCMD 0X0064
#define KEYSTA_SEND_NOTREADY 0X02
#define KEYCMD_WRITE_MODE 0X60
#define KBC_MODE 0X47

void wait_KBC_sendready(void);
void init_keyboard(void);

#endif
