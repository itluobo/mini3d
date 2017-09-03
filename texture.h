#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H
#include <stdlib.h>
// ����
typedef struct {
	int width;                  // ������
	int height;                 // ����߶�
	int nrChannels;				// ��ɫͨ��������
	float max_u, max_v;
	unsigned char * data;
} texture;

void load_texture(texture *tex, const char * path);
void device_texture_read2(const texture *tex, float u, float v, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a);

#endif // !TEXTURE_H