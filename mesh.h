#pragma once
#include "device.h"
#include "texture.h"
typedef unsigned int VBIDX;
typedef struct { float x, y, z; texcoord_t tc;} point_t2;
typedef struct { VBIDX idx; texcoord_t tc; } vertex_t2;
typedef struct {
	vertex_t2 a;
	vertex_t2 b;
	vertex_t2 c;
} triangle_t;
// Œ∆¿Ì
typedef struct {
	point_t2 *vertexs;
	size_t vertex_cnt;
	triangle_t *triangles;
	size_t triangle_cnt;
	texture* tex;
}mesh_t;

void drow_mesh(device_t *device, mesh_t* mesh);
void set_mesh_vertexs(mesh_t* mesh, point_t2 *vertexs, size_t vertex_cnt);
void set_mesh_triangles(mesh_t* mesh, triangle_t *triangles, size_t triangle_cnt);
void set_mesh_texture(mesh_t* mesh, texture* tex);