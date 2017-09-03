#ifndef D3D_H
#define D3D_H

#include <math.h>
typedef unsigned int IUINT32;
typedef struct { float m[4][4]; } matrix_t;
typedef struct { float x, y, z, w; } vector_t;
typedef vector_t point_t;

static int CMID(int x, int min, int max) { return (x < min) ? min : ((x > max) ? max : x); }

// �����ֵ��t Ϊ [0, 1] ֮�����ֵ
static float interp(float x1, float x2, float t) { return x1 + (x2 - x1) * t; }

float vector_length(const vector_t *v);
// z = x + y
void vector_add(vector_t *z, const vector_t *x, const vector_t *y);
// z = x - y
void vector_sub(vector_t *z, const vector_t *x, const vector_t *y);
// ʸ�����
float vector_dotproduct(const vector_t *x, const vector_t *y);
// ʸ�����
void vector_crossproduct(vector_t *z, const vector_t *x, const vector_t *y);
// ʸ����ֵ��tȡֵ [0, 1]
void vector_interp(vector_t *z, const vector_t *x1, const vector_t *x2, float t);
// ʸ����һ��
void vector_normalize(vector_t *v);
// c = a + b
void matrix_add(matrix_t *c, const matrix_t *a, const matrix_t *b);
// c = a - b
void matrix_sub(matrix_t *c, const matrix_t *a, const matrix_t *b);
// c = a * b
void matrix_mul(matrix_t *c, const matrix_t *a, const matrix_t *b);
// c = a * f
void matrix_scale(matrix_t *c, const matrix_t *a, float f);
// y = x * m
void matrix_apply(vector_t *y, const vector_t *x, const matrix_t *m);
void matrix_set_identity(matrix_t *m);
void matrix_set_zero(matrix_t *m);
// ƽ�Ʊ任
void matrix_set_translate(matrix_t *m, float x, float y, float z);
// ���ű任
void matrix_set_scale(matrix_t *m, float x, float y, float z);
// ��ת����
void matrix_set_rotate(matrix_t *m, float x, float y, float z, float theta);
// ������������������굽�ֲ������ת����
void matrix_set_lookat(matrix_t *m, const vector_t *eye, const vector_t *at, const vector_t *up);
// D3DXMatrixPerspectiveFovLH
void matrix_set_perspective(matrix_t *m, float fovy, float aspect, float zn, float zf);

//=====================================================================
// ����任
//=====================================================================
typedef struct {
	matrix_t world;         // ��������任
	matrix_t view;          // ��Ӱ������任
	matrix_t projection;    // ͶӰ�任
	matrix_t transform;     // transform = world * view * projection
	float w, h;             // ��Ļ��С
}	transform_t;
// ������£����� transform = world * view * projection
void transform_update(transform_t *ts);
// ��ʼ����������Ļ����
void transform_init(transform_t *ts, int width, int height);
// ��ʸ�� x ���� project 
void transform_apply(const transform_t *ts, vector_t *y, const vector_t *x);
// ����������ͬ cvv �ı߽�������׶�ü�
int transform_check_cvv(const vector_t *v);
// ��һ�����õ���Ļ����
void transform_homogenize(const transform_t *ts, vector_t *y, const vector_t *x);

//=====================================================================
// ���μ��㣺���㡢ɨ���ߡ���Ե�����Ρ���������
//=====================================================================
typedef struct { float r, g, b; } color_t;
typedef struct { float u, v; } texcoord_t;
typedef struct { point_t pos; texcoord_t tc; color_t color; float rhw; } vertex_t;

typedef struct { vertex_t v, v1, v2; } edge_t;
typedef struct { float top, bottom; edge_t left, right; } trapezoid_t;
typedef struct { vertex_t v, step; int x, y, w; } scanline_t;

void vertex_rhw_init(vertex_t *v);
void vertex_interp(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float t);
void vertex_division(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float w);
void vertex_add(vertex_t *y, const vertex_t *x);
// �������������� 0-2 �����Σ����ҷ��غϷ����ε�����
int trapezoid_init_triangle(trapezoid_t *trap, const vertex_t *p1,
	const vertex_t *p2, const vertex_t *p3);
// ���� Y ��������������������������� Y �Ķ���
void trapezoid_edge_interp(trapezoid_t *trap, float y);
// �����������ߵĶ˵㣬��ʼ�������ɨ���ߵ����Ͳ���
void trapezoid_init_scan_line(const trapezoid_t *trap, scanline_t *scanline, int y);
#endif // !D3D_H