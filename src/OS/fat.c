#include "fat.h"

void file_readfat(int *fat, unsigned char *img)
{
  //将磁盘映像中的FAT解压缩
  //ab cd ef -> dab efc
  int i, j = 0;
  for (i = 0; i < 2880; i += 2) {
    fat[i + 0] = (img[j + 0]    | img[j + 1] << 8) & 0xfff;
    fat[i + 1] = (img[j + 1]>>4 | img[j + 2] << 4) & 0xfff;
    j += 3;
  }
  return;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
  int i;
  for (;;) {
    //小于512的情况，不需要FAT也可以正常使用
    if (size <= 512) {
      for (i = 0; i < size; i++) {
	buf[i] = img[clustno * 512 + i];
      }
      break;
    }
    
    //大于512字节
    for (i = 0; i < 512; i++) {
      buf[i] = img[clustno * 512 + i];
    }
    size -= 512;
    buf += 512;
    clustno = fat[clustno];//从FAT描述表中取得扇区数
  }
  return;
}

