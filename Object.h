#pragma once
#include "Render.h"
#include "device.h"

typedef struct {
	render_t* render;
	object_t* childs;
	size_t childCnt;
}object_t;

void object_init(object_t* obj, size_t childCnt);
void object_load_model(object_t* obj, std::string path, std::string dir);