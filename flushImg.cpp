#include <cstring>
#include "misc.h"
#include "flushImg.h"

static RGB rgbTicks[11];
static int cntWaitFlush;
static uint16_t waitTblIdx;
static uint16_t baseHue;
static uint16_t actPos;
static bool bDarkMode;

static void setTicks(RGB *ptd, RGB *pts)
{
	for(int n = 0; 60 > n; n = n + 5) {
		ptd[n] = *pts++;
	}
}

void initFlushImg(void)
{
	memset(rgbTicks, 0, sizeof(rgbTicks));
	cntWaitFlush = 0;
	waitTblIdx = 0;
	baseHue = 0;
	actPos = 0;
	bDarkMode = false;
}

void setFlushImgDarkMode(bool b)
{
	bDarkMode = b; 
}

bool setFlushImg(RGB *pt)
{
	const uint8_t waitTbl[] = {
		42,35,29,25,22,19,17,15,13,12,
		11,9,9,8,7,7,6,6,5,
		5,5,4,4,4,3,3,3,3,
		3,3,2,2,2,2,2,2,2,
		2,2,2,2,1,1,1,1,1,

		//1,1,1,1,1,1,1,1,1,
		//1,1,1,1,1,1,1,1,1,

		1,1,1,1,1,2,2,2,2,
		2,2,2,2,2,2,2,3,3,
		3,3,3,3,4,4,4,5,5,
		5,6,6,7,7,8,9,9,11,
		12,13,15,17,19,22,25,29,35,42,
		
		//48, 48, 48,
	};

	const uint8_t vTbl[] = {
	 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	 9, 8, 7, 6, 5, 4, 3, 2, 1,
	};

	int n, pos;
	HSV hsv;
	bool res;

	if(0 >= cntWaitFlush) {
		if(sizeof(waitTbl)/sizeof(waitTbl[0]) <= waitTblIdx) {
			waitTblIdx = 0;
		}
		cntWaitFlush = waitTbl[waitTblIdx++];
		memset(pt, 0, LEN_LED_BUF*sizeof(RGB));
		setTicks(pt, rgbTicks);
		baseHue++;
		if(MAX_HUE <= baseHue) {
			baseHue = 0;
		}
		pos = actPos;
		for(n = 0; sizeof(vTbl) > n; n++) {
			if(LEN_LED_BUF <= pos) {
				pos = 0;
			}
			hsv.h = baseHue;
			hsv.s = FLUSH_IMG_SATURATION;
			hsv.v = vTbl[n];
			hsv2rgb(&pt[pos], &hsv);
			if(0 == n && 0 == pos % 5) {
				rgbTicks[pos/5] = pt[pos];
			}
			if(bDarkMode) {
				hsv.v = vTbl[n] / 3;
				hsv2rgb(&pt[pos], &hsv);
			}
			pos++;
		}
		actPos++;
		if(LEN_LED_BUF <= actPos) {
			actPos = 0;
		}
		res = true;
	}
	else {
		res = false;
	}
	cntWaitFlush--;
	return res;
}

