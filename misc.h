#include <stdint.h>
#ifndef _RGB_
#define _RGB_
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RGB;
#endif

#ifndef _HSV_
#define _HSV_
typedef struct {
	unsigned short h;
	unsigned char s;
	unsigned char v;
} HSV;
#endif

#define MAX_HUE					(360)
#define FLUSH_IMG_SATURATION	(246)
#define CLOCK_IMG_SATURATION	(246)

void hsv2rgb(RGB *pRgb, HSV *pHsv);
int convRgb2SpiBitStream(uint8_t *pDst, RGB *pRgb, int len);
