#ifndef __MATRIX_H
#define __MATRIX_H

#define RED    0
#define GREEN  1
#define YELLOW 2

#define VSLOW  500
#define SLOW   150
#define MID    100
#define FAST   50
#define VFAST  25

#define FADE   4
#define LEFT   3
#define UP	   1
#define DOWN   2
#define NONE   0


#define BRIGHT 44  // яркость (пин 44 - ШИМ)
#define LightSENS A7 // датчик освещенности

class MaTrix
{
	public:
		MaTrix();
		void init();
		void clearLed();
		void clearShadow();
		void setFont(unsigned char *Font);
		void printArray();
		void printShadow();
		void printChar(unsigned char sym, byte pos, byte color);
		//void printString(String s, byte pos, byte color); //, unsigned char *Font, char effect=0, byte speed = MID);
		void printString(String s, byte pos, byte color, unsigned char *Font, char effect=0, int speed = MID);
		void printRunningString(String s, byte color, unsigned char *Font, int speed = MID);
		void printStr(unsigned char *s, byte pos, byte color);
		void printCharShadow(unsigned char sym, byte pos, byte color);
		void printStringShadow(String s, byte pos, byte color);
		void printStrShadow(unsigned char *s, byte pos, byte color);
		void brightness(byte brightLevel);
		void abort();
		int getBrightness();
	protected:
		int matrixBrightness;
		
};

byte getByte(byte row, byte col);

void code();

#endif
