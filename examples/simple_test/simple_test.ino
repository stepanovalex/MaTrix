#include <SPI.h>
#include <MaTrix.h>

// Матрица
MaTrix mymatrix;

extern unsigned char font5x8[];
extern unsigned char font6x8[];
extern unsigned char digit6x8bold[];
extern unsigned char digit6x8future[];
extern byte array[8][8];
extern byte shadow[8][8];

void setup(){
  // инициализация 
  mymatrix.init();
  // очистим матрицу
  mymatrix.clearLed();
  // установим максимальную яркость
  mymatrix.brightness(255);
  // печать строки "This" с 4 позиции, красным цветом, шрифт 6х8, сдвиг влево, средняя скорость
  mymatrix.printString("This", 4, RED, font6x8, LEFT, MID);
  // печать строки "is" с 3 позиции, зеленым цветом, шрифт 6х8, сдвиг вверх, медленно
  mymatrix.printString("is", 3, GREEN, font6x8, UP, SLOW);
  // печать строки "a" с 2 позиции, желтым цветом, шрифт 6х8, сдвиг вниз, медленно
  mymatrix.printString("a", 2, YELLOW, font6x8, DOWN, SLOW);
  // печать бегущей строки "MaTrix test!" красным цветом, шрифт 6х8, быстро
  mymatrix.printRunningString("MaTrix test!", RED, font6x8, FAST);
  
  // теперь по-русски (коды кириллических символов смотри в файле /Matrix/fonts.c)
  mymatrix.printRunningString("MaTrix \xA3""o\x99\x99""ep\x9B\x9D\x97""ae\xA4"" pycc\x9F\x9D\x9E"" \xAF\x9C\xAB\x9F"" \x9D"" pa\x9C\xA2\xAB""e \xA8""p\x9D\xA5\xA4\xAB"":", GREEN, font6x8, FAST);

  // демонстрация разных шрифтов
  mymatrix.printRunningString("0123456789", RED, font6x8, FAST);
  mymatrix.printRunningString("0123456789", YELLOW, digit6x8bold, FAST);
  mymatrix.printRunningString("0123456789", GREEN, digit6x8future, FAST);
}

void loop(){
}

void code(){
  // функция обязательно должна быть (не удалять)
  // в нее размещать обработчик команд ИК, RF и т.п.
}