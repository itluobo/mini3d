#pragma once
#include "texture.h"
#include "D3D.h"
//=====================================================================
// 渲染设备
//=====================================================================
typedef struct {
	transform_t transform;      // 坐标变换器
	int width;                  // 窗口宽度
	int height;                 // 窗口高度
	IUINT32 **framebuffer;      // 像素缓存：framebuffer[y] 代表第 y行
	float **zbuffer;            // 深度缓存：zbuffer[y] 为第 y行指针
	//IUINT32 **texture;          // 纹理：同样是每行索引
	int tex_width;              // 纹理宽度
	int tex_height;             // 纹理高度
	float max_u;                // 纹理最大宽度：tex_width - 1
	float max_v;                // 纹理最大高度：tex_height - 1
	texture* tex;				// 纹理
	int render_state;           // 渲染状态
	IUINT32 background;         // 背景颜色
	IUINT32 foreground;         // 线框颜色
}	device_t;

#define RENDER_STATE_WIREFRAME      1		// 渲染线框
#define RENDER_STATE_TEXTURE        2		// 渲染纹理
#define RENDER_STATE_COLOR          4		// 渲染颜色

// 设备初始化，fb为外部帧缓存，非 NULL 将引用外部帧缓存（每行 4字节对齐）
void device_init(device_t *device, int width, int height, void *fb);
// 删除设备
void device_destroy(device_t *device);
// 清空 framebuffer 和 zbuffer
void device_clear(device_t *device, int mode);
void device_pixel(device_t *device, int x, int y, IUINT32 color);
// 绘制线段
void device_draw_line(device_t *device, int x1, int y1, int x2, int y2, IUINT32 c);
void set_texture(device_t *device, texture *tex);
// 绘制扫描线
void device_draw_scanline(device_t *device, scanline_t *scanline);
// 主渲染函数
void device_render_trap(device_t *device, trapezoid_t *trap);
// 根据 render_state 绘制原始三角形
void device_draw_primitive(device_t *device, const vertex_t *v1,
	const vertex_t *v2, const vertex_t *v3);
