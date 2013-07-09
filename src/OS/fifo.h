#ifndef _FIFO_H
#define _FIFO_H
//#include "mtask.h"

#define FLAGS_OVERRUN 0
struct TASK;
struct FIFO32 {
  unsigned int *buf;//fifo buffer pointer
  int p;//下一个数据写入的位置
  int q;//下一个数据读取的位置
  int size;//缓冲区总的长度
  int free;//没有数据的长度
  int flags;//

  struct TASK *task;//fifo相关的task
};

void fifo32_init(struct FIFO32 *fifo, int size, unsigned int *buf, struct TASK *task);
unsigned int fifo32_put(struct FIFO32 *fifo, unsigned int data);
unsigned int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

#endif
