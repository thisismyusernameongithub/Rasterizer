#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>	//sin/cos /M_PI
#include <string.h> //memcpy
#include <pthread.h>
#include <stdatomic.h>
#include <windows.h>
//If compiling with emscripten we need this header
#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

//Define some macros needed for emscripten so we don't get any errors with other compilers
#ifndef __EMSCRIPTEN__
	#define EMSCRIPTEN_KEEPALIVE
	#define EM_ASM_(...)
	#define EM_ASM(...)
	#define emscripten_run_script
#endif


#include "window.h"


#ifndef APP_NAME //These should be defined in the makefile
	#define APP_NAME "Default"
	#define APP_VER_MAJOR 0
	#define APP_VER_MINOR 0
	#define APP_VER_BUILD 0
#endif




Layer botLayer;
Layer topLayer;





typedef struct{
	union{
		struct{
			int x;
			int y;
			int z;
		};
		int array[3];
	};
}vec3i_t;

typedef struct{
	vec3f_t vertices[3];
	vec2f_t textureCoords[3];
	vec3f_t normals[3];
}Face;

typedef struct{
	struct{
		int vertices;
		int textureCoords;
		int normals;
		int faces;
	}no;
	Face* faces;
}Model;

vec3f_t testVert[3] = {
	[0] = {.x = 0.0, .y = 0.0, .z = 0.0},
	[1] = {.x = 1.0, .y = 0.0, .z = 0.0},
	[2] = {.x = 0.0, .y = 1.0, .z = 0.0}
};

Face testFaces[1];

Model testModel;

Model loadModel(char* path){
	FILE* file;
	static int maxLineLength = 256;
	char line[maxLineLength];

	Model model;

	//Open file
	file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s.\n", path);
        exit(1);
    }else{
		printf("File loaded %s.\n", path);
	}

	//Count the number of vertices
	model.no.vertices = 0;
	model.no.faces = 0;
	model.no.textureCoords = 0;
	model.no.normals = 0;
	while(fgets(line, maxLineLength, file))
	{
		if(line[0] == 'v')
		{
			if(line[1] == 't')
			{
				model.no.textureCoords++;
			}
			else if(line[1] == 'n')
			{
				model.no.normals++;
			}
			else
			{
				model.no.vertices++;
			}
		}
		else if(line[0] == 'f')
		{
			model.no.faces++;
		}
	}

	rewind(file);

	//Allocate space for all the stuff
	vec3f_t* vertices = malloc((model.no.vertices+1) * sizeof(vec3f_t));
	vec3f_t* normals = malloc((model.no.normals+1) * sizeof(vec3f_t));
	vec2f_t* textureCoords = malloc((model.no.textureCoords+1) * sizeof(vec2f_t));
	Face* faces = malloc((model.no.faces) * sizeof(Face));

	//Read the file again to copy the stuff into the buffers
	int index_tex = 1;
	int index_normal = 1;
	int index_vert = 1;
	int index_face = 0;
	while(fgets(line, maxLineLength, file))
	{
		
		if(line[0] == 'v')
		{
			if(line[1] == 't')
			{
				sscanf(line, "vt %f %f", &(textureCoords[index_tex].x), &(textureCoords[index_tex].y));
				// printf("vt %f %f \n", textureCoords[index_tex].x, textureCoords[index_tex].y);
				index_tex++;
			}
			else if(line[1] == 'n')
			{
				sscanf(line, "vn %f %f %f", &(normals[index_normal].x), &(normals[index_normal].y), &(normals[index_normal].z));
				// printf("vn %f %f %f\n", normals[index_normal].x, normals[index_normal].y, normals[index_normal].z);
				index_normal++;
			}
			else
			{
				sscanf(line, "v %f %f %f", &(vertices[index_vert].x), &(vertices[index_vert].y), &(vertices[index_vert].z));
				// printf("v %f %f %f\n", vertices[index_vert].x, vertices[index_vert].y, vertices[index_vert].z);
				index_vert++;
			}
		}
		else if(line[0] == 'f')
		{
			int v0, v1, v2, vt0, vt1, vt2, vn0, vn1, vn2 = 0;
			int result = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v0, &vt0, &vn0, &v1, &vt1, &vn1, &v2, &vt2, &vn2);
			if (result == 9) {
				// printf("Face vertices: %d/%d/%d %d/%d/%d %d/%d/%d\n", v0, vt0, vn0, v1, vt1, vn1, v2, vt2, vn2);
			} else {
				printf("Error parsing face data.\n");
				exit(0);
			}

			faces[index_face].vertices[0] = vertices[v0];
			faces[index_face].vertices[1] = vertices[v1];
			faces[index_face].vertices[2] = vertices[v2];
			faces[index_face].normals[0] = normals[vn0];
			faces[index_face].normals[1] = normals[vn1];
			faces[index_face].normals[2] = normals[vn2];
			faces[index_face].textureCoords[0] = textureCoords[vt0];
			faces[index_face].textureCoords[1] = textureCoords[vt1];
			faces[index_face].textureCoords[2] = textureCoords[vt2];
			index_face++;
		}
	}


	model.faces = faces;

	free(vertices);
	free(normals);
	free(textureCoords);

	fclose(file);
	return model;
}

argb_t red = rgb(255,0,0);
argb_t white = rgb(255,255,255);
argb_t green = rgb(0,255,0);
argb_t blue = rgb(0,0,255);

// Calculate the dot product of two vec3f_t vectors
float dotProduct(vec3f_t v1, vec3f_t v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// Function to swap two vec2i_t points
void swapVec2i(vec2i_t *a, vec2i_t *b) {
    vec2i_t temp = *a;
    *a = *b;
    *b = temp;
}

void drawTriangle(Layer layer, vec2i_t p0, vec2i_t p1, vec2i_t p2, argb_t color){
	vec2i_t P0, P1, P2;
	//Find lowest point
	if (p0.y>p1.y) swapVec2i(&p0, &p1); 
    if (p0.y>p2.y) swapVec2i(&p0, &p2); 
    if (p1.y>p2.y) swapVec2i(&p1, &p2);
	//Find slope of left and right side of triangle
	int y = p0.y;
	int yMid = p1.y;
	int yTop = p2.y;
	float lim1 = p0.x;
	float lim2 = p0.x;
	float slope1 = (float)(p1.x - p0.x) / (float)(p1.y - p0.y);
	float slope2 = (float)(p2.x - p0.x) / (float)(p2.y - p0.y);

	//Draw line by line, increasing width by slope for each line.
	for(; y < yMid; y++){
		lim1 += slope1;
		lim2 += slope2;
		drawLine(layer, lim1, y, lim2, y, color);
		// window_run(); Sleep(1);
	}
	
	lim1 = p1.x; //lim1 should already be p1.x EXCEPT for the corner case where the two lowest points have the exact same y value.

	//When first point is met, recalculate slope from that point to the last
	slope1 = (float)(p2.x - p1.x) / (float)(p2.y - p1.y);
	//Draw rest of triangle
	for(; y < yTop; y++){
		lim1 += slope1;
		lim2 += slope2;
		drawLine(layer, lim1, y, lim2, y, color);
		// window_run(); Sleep(1);
	}
	// drawPoint(layer, p0.x, p0.y, red);
	// drawPoint(layer, p1.x, p1.y, green);
	// drawPoint(layer, p2.x, p2.y, blue);
}

vec3f_t light_dir = {0.f, 0.f, 1.f};

// Rotate a point around the origin
void rotatePoint(vec2i_t *point, float angle) {
    // Translate the point to the origin
    int x_translated = point->x;
    int y_translated = point->y;

    // Rotate the translated point around the origin
    float x_rotated = x_translated * cos(angle) - y_translated * sin(angle);
    float y_rotated = x_translated * sin(angle) + y_translated * cos(angle);

    // Translate the rotated point back to its original position
    point->x = round(x_rotated);
    point->y = round(y_rotated);
}


static int mainLoop()
{
	char titleString[100];
	sprintf(titleString, "fps: %.2f, ms: %.2f", window.time.fps, 1000.f*window.time.dTime);
	if(window.time.tick.ms100) window_setTitle(titleString);

	
	clearLayer(botLayer);
	clearLayer(topLayer);

	drawText(topLayer, 10, 10, printfLocal("fps: %.2f, ms: %.2f", window.time.fps, 1000.f*window.time.dTime));
    
	// Define the points
    vec2i_t p0 = { 10, 87};
    vec2i_t p1 = { 46, 155};
    vec2i_t p2 = { 49, 87};

    // Define the rotation angle based on the current time
    static float angle = 0.f;
	angle += (float)mouse.dWheel / (100.f + key.shiftLeft*100.f); // Scale time to control rotation speed

    // Rotate each point around the center
    // rotatePoint(&p0, angle);
    // rotatePoint(&p1, angle);
    // rotatePoint(&p2, angle);

	// p0.x += 100;
	// p0.y += 100;
	// p1.x += 100;
	// p1.y += 100;
	// p2.x += 100;
	// p2.y += 100;

	// drawText(topLayer, 10, 40, printfLocal("%d %d, %d %d, %d %d", p0.x, p0.y, p1.x, p1.y, p2.x, p2.y));
    

	// drawTriangle(botLayer, p0, p1, p2, white);
	// drawTriangle(botLayer, p0, p2, p1, white);
	// drawTriangle(botLayer, p1, p0, p2, green);
	// drawTriangle(botLayer, p1, p2, p0, red);
	// drawTriangle(botLayer, p2, p0, p1, white);
	// drawTriangle(botLayer, p2, p1, p0, green);

	for(int y = 0; y < botLayer.h; y++){
		for(int x = 0; x < botLayer.w; x++){
			botLayer.frameBuffer[x + y * botLayer.w] = rgb(255 * y / botLayer.h, 50, 50);	
		}	
	}

	srand(1);
	for(int face = 0; face < testModel.no.faces; face++){
		// for(int i = 0; i < 3; i++){
		// 	// float x0 = testModel.vertices[testModel.faces[face].vert[i] - 1].x;
		// 	// float y0 = testModel.vertices[testModel.faces[face].vert[i] - 1].y;
		// 	// float x1 = testModel.vertices[(testModel.faces[face].vert[i])%3].x;
		// 	// float y1 = testModel.vertices[(testModel.faces[face].vert[i])%3].y;
		// 	float x0 = testModel.faces[face].vertices[i].x;
		// 	float y0 = testModel.faces[face].vertices[i].y;
		// 	float x1 = testModel.faces[face].vertices[(i+1)%3].x;
		// 	float y1 = testModel.faces[face].vertices[(i+1)%3].y;
		// 	// printf("%f %f \n",x,y);
		// 	x0 = (botLayer.w / 2) + x0 * (botLayer.w / 2);
		// 	y0 = (botLayer.h / 2) + y0 * (botLayer.h / 2);
		// 	x1 = (botLayer.w / 2) + x1 * (botLayer.w / 2);
		// 	y1 = (botLayer.h / 2) + y1 * (botLayer.h / 2);
		// 	drawLine(botLayer, x0, y0, x1, y1, white);
		// }
		float x0 = testModel.faces[face].vertices[0].x;
		float y0 = testModel.faces[face].vertices[0].y;
		float x1 = testModel.faces[face].vertices[1].x;
		float y1 = testModel.faces[face].vertices[1].y;
		float x2 = testModel.faces[face].vertices[2].x;
		float y2 = testModel.faces[face].vertices[2].y;
		// printf("%f %f \n",x,y);
		x0 = (botLayer.w / 2) + x0 * (botLayer.w / 2);
		y0 = (botLayer.h / 2) + y0 * (botLayer.h / 2);
		x1 = (botLayer.w / 2) + x1 * (botLayer.w / 2);
		y1 = (botLayer.h / 2) + y1 * (botLayer.h / 2);
		x2 = (botLayer.w / 2) + x2 * (botLayer.w / 2);
		y2 = (botLayer.h / 2) + y2 * (botLayer.h / 2);
		vec2i_t p0 = {x0, y0};
		vec2i_t p1 = {x1, y1};
		vec2i_t p2 = {x2, y2};
		float intensity = dotProduct(light_dir, testModel.faces[face].normals[0]); //Assume all normals are the same for face
		argb_t color = rgb(intensity * 150,intensity * 150,intensity * 150);
		if(intensity > 1.f) {
			// printf("%f\n", intensity);
		}
		if(intensity > 0.f)
		{
			drawTriangle(botLayer, p0, p1, p2, color);
		}
	}

	

	// Invert Y axis by flipping framebuffer before drawing.
	static argb_t tempBuffer[512*512];
	for(int y = 0; y < botLayer.h; y++){
		for(int x = 0; x < botLayer.w; x++){
			tempBuffer[x + y * botLayer.w] = botLayer.frameBuffer[x + (botLayer.h - 1 - y)  * botLayer.w];
		}	
	}
	memcpy(botLayer.frameBuffer, tempBuffer, window.drawSize.w*window.drawSize.h*sizeof(argb_t));

	if(key.ESC){
		window.closeWindow = true;
	}

	return window_run();
}


int main()
{
	// Disable console buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	system("CHCP 65001"); //Enable unicode characters in the terminal


	printf("%s %d.%d.%d - %s %s\n", APP_NAME, APP_VER_MAJOR, APP_VER_MINOR, APP_VER_BUILD, __DATE__, __TIME__);

	window.pos.x = 20;
	window.pos.y = 20;
	window.drawSize.w = 512;
	window.drawSize.h = 512;
	window.size.w = window.drawSize.w*2;
	window.size.h = window.drawSize.h*2;
	


	window_init();

	// init bottom layer
	botLayer = window_createLayer();
	// init top layer
	topLayer = window_createLayer();

	testModel = loadModel("./Resources/african_head.obj");

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop((void (*)(void))mainLoop, 0, 1);
#else
	while (mainLoop())
	{
	}
#endif

	return 0;
}



