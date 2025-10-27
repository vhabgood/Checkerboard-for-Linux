

#pragma once

#include "checkers_types.h"

#define ANIMATIONSLEEPTIME 4		// 5 ms per animation step
#define STEPS 12.0					// 12 number of animation steps to make
#define CLOCKHEIGHT 25

// Forward declarations for generic GUI types (to be replaced by Qt types later)
struct GenericWindowHandle; // Placeholder for HWND
struct GenericDeviceContext; // Placeholder for HDC

// Function prototypes, updated for generic types
// DWORD AnimationThreadFunc(HWND hwnd);
// int getxymetrics(double *xmetric, double *ymetric, HWND hwnd);
int printboard(GenericWindowHandle* hwnd, GenericDeviceContext* hdc, GenericDeviceContext* bmpdc, GenericDeviceContext* stretchdc, Board8x8 board);
int printsampleboard(GenericWindowHandle* hwnd, GenericDeviceContext* hdc, GenericDeviceContext* bmpdc, GenericDeviceContext* stretchdc);
void selectstone(int x, int y, GenericWindowHandle* hwnd);
void selectstones(Squarelist &squares, GenericWindowHandle* hwnd);
void updatestretchDC(GenericWindowHandle* hwnd, GenericDeviceContext* bmpdc, GenericDeviceContext* stretchdc, int size);
int updategraphics(GenericWindowHandle* hwnd);
int resizegraphics(GenericWindowHandle* hwnd);
int updateboardgraphics(GenericWindowHandle* hwnd);
void refresh_clock(GenericWindowHandle* hwnd);
int initgraphics(GenericWindowHandle* hwnd);
int diagramtoclipboard(GenericWindowHandle* hwnd);
int samplediagramtoclipboard(GenericWindowHandle* hwnd);
int setoffsets(int _offset, int _upperoffset);
void set_animation(bool state);
