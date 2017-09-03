#pragma once
#include "mesh.h"
static void conversion2point(point_t* x, point_t2* y) {
	x->w = 1;
	x->x = y->x;
	x->y = y->y;
	x->z = y->z;
}

static void read_triangle(mesh_t* mesh, VBIDX idx, vertex_t* v1, vertex_t* v2, vertex_t* v3) {
	triangle_t* triangle = &mesh->triangles[idx];
	conversion2point(&v1->pos, &mesh->vertexs[triangle->a.idx]);
	v1->tc = triangle->a.tc;
	v1->rhw = 1;
	
	conversion2point(&v2->pos, &mesh->vertexs[triangle->b.idx]);
	v2->tc = triangle->b.tc;
	v2->rhw = 1;

	conversion2point(&v3->pos, &mesh->vertexs[triangle->c.idx]);
	v3->tc = triangle->c.tc;
	v3->rhw = 1;
}

void drow_mesh(device_t *device, mesh_t* mesh) {
	set_texture(device, mesh->tex);
	vertex_t v1, v2, v3;
	for (int i = 0; i < mesh->triangle_cnt; ++i) {
		read_triangle(mesh, i, &v1, &v2, &v3);
		device_draw_primitive(device, &v1, &v2, &v3);
	}
}

void set_mesh_vertexs(mesh_t* mesh, point_t2 *vertexs, size_t vertex_cnt) {
	mesh->vertexs = vertexs;
	mesh->vertex_cnt = vertex_cnt;
}

void set_mesh_triangles(mesh_t* mesh, triangle_t *triangles, size_t triangle_cnt) {
	mesh->triangles = triangles;
	mesh->triangle_cnt = triangle_cnt;
}

void set_mesh_texture(mesh_t* mesh, texture* tex) {
	mesh->tex = tex;
}