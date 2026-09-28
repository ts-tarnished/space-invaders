#ifndef TS_SW_GRAPHICS_H
#define TS_SW_GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	int x;
	int y;
	int width;
	int height;
} TSG_Rectangle;

typedef struct  TSG_PixelArray {
	int width;
	int height;
	uint32_t *data;
} TSG_PixelArray;

void TSG_SwapPoints(int *x0, int *y0, int *x1, int *y1);
bool TSG_InRectangle(int x, int y, TSG_Rectangle rectangle);
void TSG_DrawRectangle(TSG_PixelArray *pixels, int x, int y, int width, int height, uint32_t color);
void TSG_DrawRectangleRec(TSG_PixelArray *pixels, TSG_Rectangle rectangle, uint32_t color);
void TSG_DrawRectangleRecBordered(TSG_PixelArray *pixels, TSG_Rectangle rectangle, int borderWidth, uint32_t recColor, uint32_t borderColor);
bool TSG_GetPixel(TSG_PixelArray *pixels, int x, int y, uint32_t *color);
void TSG_SetPixel(TSG_PixelArray *pixels, int x, int y, uint32_t color);
void TSG_DrawLine(TSG_PixelArray *pixels, int x0, int y0, int x1, int y1, uint32_t color);
void TSG_DrawText(TSG_PixelArray *pixels, int x, int y, char *text, uint32_t color);
void TSG_DrawTriangle(TSG_PixelArray *pixels, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void TSG_Clear(TSG_PixelArray *pixels, uint32_t color);

#endif
