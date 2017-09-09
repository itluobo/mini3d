#pragma once
#include "texture.h"
#include "D3D.h"
//=====================================================================
// 渲染设备
//=====================================================================
typedef struct {
	camera_t camera;			// 相机
	matrix_t world;				// 世界坐标变换
	int width;                  // 窗口宽度
	int height;                 // 窗口高度
	IUINT32 **framebuffer;      // 像素缓存：framebuffer[y] 代表第 y行
	float **zbuffer;            // 深度缓存：zbuffer[y] 为第 y行指针
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
