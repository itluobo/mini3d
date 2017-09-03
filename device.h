#pragma once
#include "texture.h"
#include "D3D.h"
//=====================================================================
// ��Ⱦ�豸
//=====================================================================
typedef struct {
	transform_t transform;      // ����任��
	int width;                  // ���ڿ��
	int height;                 // ���ڸ߶�
	IUINT32 **framebuffer;      // ���ػ��棺framebuffer[y] ����� y��
	float **zbuffer;            // ��Ȼ��棺zbuffer[y] Ϊ�� y��ָ��
	//IUINT32 **texture;          // ����ͬ����ÿ������
	int tex_width;              // ������
	int tex_height;             // ����߶�
	float max_u;                // ��������ȣ�tex_width - 1
	float max_v;                // �������߶ȣ�tex_height - 1
	texture* tex;				// ����
	int render_state;           // ��Ⱦ״̬
	IUINT32 background;         // ������ɫ
	IUINT32 foreground;         // �߿���ɫ
}	device_t;

#define RENDER_STATE_WIREFRAME      1		// ��Ⱦ�߿�
#define RENDER_STATE_TEXTURE        2		// ��Ⱦ����
#define RENDER_STATE_COLOR          4		// ��Ⱦ��ɫ

// �豸��ʼ����fbΪ�ⲿ֡���棬�� NULL �������ⲿ֡���棨ÿ�� 4�ֽڶ��룩
void device_init(device_t *device, int width, int height, void *fb);
// ɾ���豸
void device_destroy(device_t *device);
// ��� framebuffer �� zbuffer
void device_clear(device_t *device, int mode);
void device_pixel(device_t *device, int x, int y, IUINT32 color);
// �����߶�
void device_draw_line(device_t *device, int x1, int y1, int x2, int y2, IUINT32 c);
void set_texture(device_t *device, texture *tex);
// ����ɨ����
void device_draw_scanline(device_t *device, scanline_t *scanline);
// ����Ⱦ����
void device_render_trap(device_t *device, trapezoid_t *trap);
// ���� render_state ����ԭʼ������
void device_draw_primitive(device_t *device, const vertex_t *v1,
	const vertex_t *v2, const vertex_t *v3);
