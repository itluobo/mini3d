//=====================================================================
// 
// mini3d.c - Mini Software Render All-In-One
//
// build:
//   mingw: gcc -O3 mini3d.c -o mini3d.exe -lgdi32
//   msvc:  cl -O2 -nologo mini3d.c 
//
// history:
//   2007.7.01  skywind  create this file as a tutorial
//   2007.7.02  skywind  implementate texture and color render
//   2008.3.15  skywind  fixed a trapezoid issue
//   2015.8.09  skywind  rewrite with more comment
//   2015.8.12  skywind  adjust interfaces for clearity 
// 
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>

#include <iostream>

#include "D3D.h"
#include "texture.h"
#include "device.h"
#include "mesh.h"

extern "C"{
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "lua/lobject.h"
#include "lua/lstate.h"
}


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Assimp::Importer importer;
//=====================================================================
// Win32 窗口及图形绘制：为 device 提供一个 DibSection 的 FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// 当前键盘按下状态
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// 老的 BITMAP
unsigned char *screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR *title);	// 屏幕初始化
int screen_close(void);								// 关闭屏幕
void screen_dispatch(void);							// 处理消息
void screen_update(void);							// 显示 FrameBuffer

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);	

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// 初始化窗口并设置标题
int screen_init(int w, int h, const TCHAR *title) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0, 
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB, 
		w * h * 4, 0, 0, 0, 0 }  };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;
	
	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

int screen_close(void) {
	if (screen_dc) {
		if (screen_ob) { 
			SelectObject(screen_dc, screen_ob); 
			screen_ob = NULL; 
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) { 
		DeleteObject(screen_hb); 
		screen_hb = NULL; 
	}
	if (screen_handle) { 
		CloseWindow(screen_handle); 
		screen_handle = NULL; 
	}
	return 0;
}

static LRESULT screen_events(HWND hWnd, UINT msg, 
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void screen_dispatch(void) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}


//=====================================================================
// 主程序
//=====================================================================
vertex_t box[8] = {
	{ {  1, -1,  1, 1 }, { 1, 0 }, { 1.0f, 0.2f, 0.2f }, 1 },
	{ { -1, -1,  1, 1 }, { 0, 0 }, { 0.2f, 1.0f, 0.2f }, 1 },
	{ { -1,  1,  1, 1 }, { 0, 0 }, { 0.2f, 0.2f, 1.0f }, 1 },
	{ {  1,  1,  1, 1 }, { 1, 0 }, { 1.0f, 0.2f, 1.0f }, 1 },
	{ {  1, -1, -1, 1 }, { 1, 1 }, { 1.0f, 1.0f, 0.2f }, 1 },
	{ { -1, -1, -1, 1 }, { 0, 1 }, { 0.2f, 1.0f, 1.0f }, 1 },
	{ { -1,  1, -1, 1 }, { 0, 1 }, { 1.0f, 0.3f, 0.3f }, 1 },
	{ {  1,  1, -1, 1 }, { 1, 1 }, { 0.2f, 1.0f, 0.3f }, 1 },
};

void draw_plane(device_t *device, vertex_t* mesh, int a, int b, int c, int d) {
	vertex_t p1 = mesh[a], p2 = mesh[b], p3 = mesh[c], p4 = mesh[d];
	p1.tc.u = 0, p1.tc.v = 0, p2.tc.u = 0, p2.tc.v = 1;
	p3.tc.u = 1, p3.tc.v = 1, p4.tc.u = 1, p4.tc.v = 0;
	device_draw_primitive(device, &p1, &p2, &p3);
	device_draw_primitive(device, &p3, &p4, &p1);
}

void draw_box(device_t *device, float theta) {
	matrix_t m;
	matrix_set_rotate(&m, 0, 1, 0, theta);
	device->world = m;
	//camera_update(&device->camera);
	draw_plane(device, box, 0, 1, 2, 3);
	draw_plane(device, box, 4, 5, 6, 7);
	draw_plane(device, box, 0, 4, 5, 1);
	draw_plane(device, box, 1, 5, 6, 2);
	draw_plane(device, box, 2, 6, 7, 3);
	draw_plane(device, box, 3, 7, 4, 0);
}

point_t2 box_vertexs[8] = {
	{  1, -1,  1},
	{ -1, -1,  1},
	{ -1,  1,  1},
	{  1,  1,  1},
	{  1, -1, -1},
	{ -1, -1, -1},
	{ -1,  1, -1},
	{  1,  1, -1},
};

triangle_t box_triangles[12] = {
	{{0, {0, 0}}, {1, {0, 1}}, {2, {1, 1}}},
	{{2, {1, 1}}, {3, {1, 0}}, {0, {0, 0}}},

	{{4, {0, 0}}, {5, {0, 1}}, {6, {1, 1}}},
	{{6, {1, 1}}, {7, {1, 0}}, {4, {0, 0}}},

	{{0, {0, 0}}, {4, {0, 1}}, {5, {1, 1}}},
	{{5, {1, 1}}, {1, {1, 0}}, {0, {0, 0}}},

	{{1, {0, 0}}, {5, {0, 1}}, {6, {1, 1}}},
	{{6, {1, 1}}, {2, {1, 0}}, {1, {0, 0}}},

	{{2, {0, 0}}, {6, {0, 1}}, {7, {1, 1}}},
	{{7, {1, 1}}, {3, {1, 0}}, {2, {0, 0}}},

	{{3, {0, 0}}, {7, {0, 1}}, {4, {1, 1}}},
	{{4, {1, 1}}, {0, {1, 0}}, {3, {0, 0}}},
};

vertex_t plan[4] = {
	{ { -6, -3,  3, 1 },{ 0, 0 },{ 1.0f, 0.2f, 0.2f }, 1 },
	{ { 6, -3,  3, 1 },{ 1, 0 },{ 0.2f, 1.0f, 0.2f }, 1 },
	{ { 6,  -3,  -3, 1 },{ 1, 1 },{ 0.2f, 0.2f, 1.0f }, 1 },
	{ { -6,  -3,  -3, 1 },{ 0, 1 },{ 1.0f, 0.2f, 1.0f }, 1 },
};

void draw_ground(device_t *device) {
	matrix_t m;
	matrix_set_rotate(&m, 1, 0, 0, 0);
	device->world = m;
	//camera_update(&device->camera);
	
	vertex_t p1 = plan[0], p2 = plan[1], p3 = plan[2], p4 = plan[3];
	device_draw_primitive(device, &p1, &p2, &p3);
	device_draw_primitive(device, &p3, &p4, &p1);
}

void camera_at_zero(device_t *device, float x, float y, float z) {
	point_t eye = { x, y, z, 1 }, at = { 0, 0, 0, 1 }, up = { 0, 1, 0, 1 };
	matrix_set_lookat(&device->camera.view, &eye, &at, &up);
	camera_update(&device->camera);
}

void loadModel(mesh_t* tar, std::string path, std::string dir)
{
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	aiNode* node = scene->mRootNode;
	node = node->mChildren[0];
	std::cout << node->mNumMeshes << node->mNumChildren;
	aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
	point_t2* pos_list = (point_t2*)malloc(sizeof(point_t2) * mesh->mNumVertices);
	set_mesh_vertexs(tar, pos_list, mesh->mNumVertices);
	point_t2* pos = pos_list;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		pos->x = mesh->mVertices[i].x;
		pos->y = mesh->mVertices[i].y;
		pos->z = mesh->mVertices[i].z;
		if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			texcoord_t tc;
			tc.u = mesh->mTextureCoords[0][i].x;
			tc.v = mesh->mTextureCoords[0][i].y;
			pos->tc = tc;
		}
		else {
			pos->tc.u = 0.5f;
			pos->tc.v = 0.5f;
		}
		++pos;
	}
	triangle_t* faces = (triangle_t*)malloc(sizeof(triangle_t) * mesh->mNumFaces);
	set_mesh_triangles(tar, faces, mesh->mNumFaces);
	triangle_t* triangle = faces;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		triangle->a.idx = face.mIndices[0];
		triangle->a.tc = pos_list[triangle->a.idx].tc;
		triangle->b.idx = face.mIndices[1];
		triangle->b.tc = pos_list[triangle->b.idx].tc;
		triangle->c.idx = face.mIndices[2];
		triangle->c.tc = pos_list[triangle->c.idx].tc;
		++triangle;
	}
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	for (unsigned int i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
		texture* tex = (texture*)malloc(sizeof(texture));
		//std::cout << "load tex " << dir.append(str.C_Str()).c_str() << std::endl;
		load_texture(tex, dir.append(str.C_Str()).c_str());
		set_mesh_texture(tar, tex);
	}
	std::cout << "loadModel suc " << path << "faces"<< mesh->mNumFaces << std::endl;
}

void test_lua(){
	// test lua
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dostring(L, "print(\"hello lua\")");
	luaL_dostring(L, "require(\"res.LuaScrips.hello\")");
}

int main(void)
{
	test_lua();
	device_t device;
	texture ground_tex, box_tex;
	mesh_t box_mesh;
	load_texture(&ground_tex, "res/images/hello.png");;
	load_texture(&box_tex, "res/images/container2.png");
	//load_texture(&box_tex, "res/objects/cyborg/cyborg_diffuse.png");
	//set_mesh_vertexs(&box_mesh, box_vertexs, 8);
	//set_mesh_triangles(&box_mesh, box_triangles, 12);
	set_mesh_texture(&box_mesh, &box_tex);
	int states[] = { RENDER_STATE_TEXTURE, RENDER_STATE_COLOR, RENDER_STATE_WIREFRAME };
	int indicator = 0;
	int kbhit = 0;
	float alpha = 3.14f;
	float pos = 5;

	TCHAR *title = _T("Mini3d (software render tutorial) - ")
		_T("Left/Right: rotation, Up/Down: forward/backward, Space: switch state");

	if (screen_init(800, 600, title)) 
		return -1;
	loadModel(&box_mesh, "res/objects/Mandroid/Mandroid.obj", "res/objects/Mandroid/Texture/");
	device_init(&device, 800, 600, screen_fb);
	camera_at_zero(&device, 0, 5, -1);
	//init_texture2(&device);
	device.render_state = RENDER_STATE_TEXTURE;

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch();
		device_clear(&device, 1);
		camera_at_zero(&device, 0, 3, -pos);
		
		if (screen_keys[VK_UP]) pos -= 0.01f;
		if (screen_keys[VK_DOWN]) pos += 0.01f;
		if (screen_keys[VK_LEFT]) alpha += 0.05f;
		if (screen_keys[VK_RIGHT]) alpha -= 0.05f;

		if (screen_keys[VK_SPACE]) {
			if (kbhit == 0) {
				kbhit = 1;
				if (++indicator >= 3) indicator = 0;
				device.render_state = states[indicator];
			}
		}	else {
			kbhit = 0;
		}
		set_texture(&device, &ground_tex);
		draw_ground(&device);
		//set_texture(&device, &box_tex);
		//draw_box(&device, alpha);
		matrix_t m;
		matrix_set_rotate(&m, 0, 1, 0, alpha);
		device.world = m;
		//camera_update(&device.camera);
		drow_mesh(&device, &box_mesh);
		screen_update();
		Sleep(1);
	}
	return 0;
}
