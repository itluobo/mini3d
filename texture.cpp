#pragma once
#include "texture.h"
#include "D3D.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void load_texture(texture *tex, const char * path) {
	int width, height, nrChannels;
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) {
		tex->height = height;
		tex->width = width;
		tex->nrChannels = nrChannels;
		tex->data = (unsigned char *)malloc(width * height * nrChannels);
		tex->max_u = (float)(width - 1);
		tex->max_v = (float)(height - 1);
		memcpy(tex->data, data, width * height * nrChannels);

		printf("load texture[%s] success width:%d height:%d nrChannels:%d\n", path, width, height, nrChannels);
	}
	else {
		printf("load texture failed %s\n", path);
	}
}

// 根据坐标读取纹理
void device_texture_read2(const texture *tex, float u, float v, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a) {
	int x, y;
	u = u * tex->max_u;
	v = v * tex->max_v;
	x = (int)(u + 0.5f);
	y = (int)(v + 0.5f);
	x = CMID(x, 0, tex->width - 1);
	y = CMID(y, 0, tex->height - 1);
	unsigned char *pos = tex->data + y * tex->width * tex->nrChannels + x * tex->nrChannels;
	*r = pos[0];
	*g = pos[1];
	*b = pos[2];
	if (tex->nrChannels > 3) {
		*a = pos[3];
	}
	else {
		*a = 1;
	}
}

