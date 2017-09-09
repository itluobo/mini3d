#pragma once
#include "texture.h"
#include "mesh.h"
#include "device.h"
#include <assimp/scene.h>

typedef struct
{
	texture* tex;
}materials_t;

typedef struct{
	materials_t* material;
	mesh_t* mesh;
}render_t;

void load_model(render_t* render, const aiScene* scene, const aiNode* node);
void drow_render(device_t *device, render_t* mesh);