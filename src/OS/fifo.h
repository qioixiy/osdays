#ifndef _FIFO_H
#define _FIFO_H

#define FLAGS_OVERRUN 0

struct FIFO8 {
  unsigned char *buf;//fifo buffer pointer
  int p;//下一个数据写入的位置
  int q;//下一个数据读取的位置
  int size;//缓冲区总的长度
  int free;//没有数据的长度
  int flags;//
};

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

#endif
