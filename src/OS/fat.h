#ifndef _FAT_H
#define _FAT_H

void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);

#endif
