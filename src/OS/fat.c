#include "fat.h"

void file_readfat(int *fat, unsigned char *img)
{
  //������ӳ���е�FAT��ѹ��
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
    //С��512�����������ҪFATҲ��������ʹ��
    if (size <= 512) {
      for (i = 0; i < size; i++) {
	buf[i] = img[clustno * 512 + i];
      }
      break;
    }
    
    //����512�ֽ�
    for (i = 0; i < 512; i++) {
      buf[i] = img[clustno * 512 + i];
    }
    size -= 512;
    buf += 512;
    clustno = fat[clustno];//��FAT��������ȡ��������
  }
  return;
}
