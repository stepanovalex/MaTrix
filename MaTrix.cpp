#include "Arduino.h"
#include "MaTrix.h"
#include <SPI.h>


byte array[8][8] = { // Массив из 64 байт
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 7
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 6
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 5
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 4
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 3
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 2
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 1
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000}   // строка 0
  // red3      green3      red2      green2      red1      green1       red0      green0
};

byte shadow[8][8] = { // Массив из 64 байт
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 7
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 6
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 5
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 4
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 3
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 2
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},  // строка 1
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000}   // строка 0
  // red3      green3      red2      green2      red1      green1       red0      green0
};

boolean longStringFlag = false;

/* Адресные пины демультиплексора:
Демультиплексор  Ардуино
  A0               2
  A1               3
  A2               4
*/

byte row = 0;          // строка матрицы, с которой работаем в текущий момент
byte addrmask = B00000111; // маска адреса по которой будем работать (пины 2,3,4 - на адресные входы демультиплексора)

unsigned char *pFont;

/*
//Пин SS (любой, у меня 7) подключен к ST_CP входу 74HC595
//Пин SCK (13) подключен к SH_CP входу 74HC595
//Пин MOSI (11) подключен к DS входу 74HC595
*/

MaTrix::MaTrix(){
}

void MaTrix::init(){
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();                   // инициализируем SPI
  //SPI.setBitOrder(MSBFIRST);
  
  // ШИМ для регулирования яркости матрицы
  //TCCR5B = B00001011;  // ~1кГц 
  //TCCR5B = B00000010;  // ~4кГц 
  TCCR5B = B00001010;  // ~8кГц 
  
  DDRL = DDRL | B11110111;       // инициализируем порт L на OUTPUT (7, 5, 4,3,2 пины) 
  //Serial.begin(9600);
  /*
  pinMode(42, OUTPUT);  // SS регистров сдвига
  pinMode(43, OUTPUT);  // гашение (E1, E2)
  pinMode(44, OUTPUT);  // яркость PWM (E3)
  pinMode(47, OUTPUT);  // адрес A2
  pinMode(48, OUTPUT);  // адрес A1
  pinMode(49, OUTPUT);  // адрес A0
  */

  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 2000;              // compare match register 16MHz/8/800Hz
                             // 100Гц - для всего дисплея, но отображаем за раз 1 из 8 строк
  TCCR1B |= (1 << WGM12);    // CTC mode
  TCCR1B |= (1 << CS11);     // 8 prescaler 
  TIMSK1 |= (1 << OCIE1A);   // enable timer compare interrupt
  interrupts();              // enable all interrupts
}

void MaTrix::brightness(byte brightLevel) {
	analogWrite(BRIGHT, brightLevel);
}

// печатаем символ sym в позиции pos (от 0 до 5, нумерация справа-налево)
void MaTrix::printChar(unsigned char sym, byte pos, byte color) {
  if ((sym>=pgm_read_byte(pFont+2))&&(sym<=(pgm_read_byte(pFont+2)+pgm_read_byte(pFont+3)))&&(sym>=pgm_read_byte(pFont+2))&&(pos>=0)&&(pos<=(pgm_read_byte(pFont+4)-1))) { // проверка на то, что находимся в рамках шрифта
    // posMr - номер байта для "младшей" физической матрицы, на которой производится отображение
    // posMl - номер байта для "старшей" физической матрицы, на которой производится отображение
    // fontM - текущий байт в шрифте для нужного символа
    // posP - смещение для получения номера младшей матрицы, участвующей в отображении
    int posMr, posMl, fontM, posP, posPl, posPf;
    // битовые маски (полная, правая и левая часть)
    byte mask, maskSr, maskSl, width, offset;
        
    posP=pos+11;  // начало данных о маладшей матрице
    posPl=posP+6; // начало данных о старшей матрице
    posPf=pos+5;  // начало данных о смещениях символа относительно начала физ.матрицы
    
    mask=0;
    width=pgm_read_byte(pFont);
    // высчитаваем маску для вывода
    for (byte m=0; m<width; m++){
      mask=mask+(1<<m);
    }

    // маска для левой и правой части символа
    offset=pgm_read_byte(pFont+posPf);
    maskSr=mask<<offset;
    maskSl=mask>>(8-offset);
    
    // номера байт для физических матриц (с учетом цвета)
    posMr=6-pgm_read_byte(pFont+posP)*2+color;
    posMl=4-pgm_read_byte(pFont+posP)*2+color;
        
    for (byte i=0; i<8; i++) {
      fontM=(sym-pgm_read_byte(pFont+2))*8+11+12+i;
      if(color<3 && color!=YELLOW) {
        // отрисовываем "младшую" матрицу
        array[i][posMr]=(array[i][posMr] & ~maskSr) | (pgm_read_byte(pFont+fontM)<<pgm_read_byte(pFont+posPf));
        
        // если нужно, отрисовывавем "старшую" матрицу
        if(pgm_read_byte(pFont+posPl) != 5) { 
          array[i][posMl]=(array[i][posMl] & ~maskSl) | (pgm_read_byte(pFont+fontM)>>(8-pgm_read_byte(pFont+posPf)));
        }
      }
      else {
        printChar(sym, pos, GREEN);
        printChar(sym, pos, RED);
      }
    }
  }
}

void MaTrix::printCharShadow(unsigned char sym, byte pos, byte color) {
  if ((sym>=pgm_read_byte(pFont+2))&&(sym<=(pgm_read_byte(pFont+2)+pgm_read_byte(pFont+3)))&&(sym>=pgm_read_byte(pFont+2))&&(pos>=0)&&(pos<=(pgm_read_byte(pFont+4)-1))) { // проверка на то, что находимся в рамках шрифта
    // posMr - номер байта для "младшей" физической матрицы, на которой производится отображение
    // posMl - номер байта для "старшей" физической матрицы, на которой производится отображение
    // fontM - текущий байт в шрифте для нужного символа
    // posP - смещение для получения номера младшей матрицы, участвующей в отображении
    int posMr, posMl, fontM, posP, posPl, posPf;
    // битовые маски (полная, правая и левая часть)
    byte mask, maskSr, maskSl, width, offset;
        
    posP=pos+11;  // начало данных о маладшей матрице
    posPl=posP+6; // начало данных о старшей матрице
    posPf=pos+5;  // начало данных о смещениях символа относительно начала физ.матрицы
    
    mask=0;
    width=pgm_read_byte(pFont);
    // высчитаваем маску для вывода
    for (byte m=0; m<width; m++){
      mask=mask+(1<<m);
    }

    // маска для левой и правой части символа
    offset=pgm_read_byte(pFont+posPf);
    maskSr=mask<<offset;
    maskSl=mask>>(8-offset);
    
    // номера байт для физических матриц (с учетом цвета)
    posMr=6-pgm_read_byte(pFont+posP)*2+color;
    posMl=4-pgm_read_byte(pFont+posP)*2+color;
        
    for (byte i=0; i<8; i++) {
      fontM=(sym-pgm_read_byte(pFont+2))*8+11+12+i;
      if(color<3 && color!=YELLOW) {
        // отрисовываем "младшую" матрицу
        shadow[i][posMr]=(shadow[i][posMr] & ~maskSr) | (pgm_read_byte(pFont+fontM)<<pgm_read_byte(pFont+posPf));
        
        // если нужно, отрисовывавем "старшую" матрицу
        if(pgm_read_byte(pFont+posPl) != 5) { 
          shadow[i][posMl]=(shadow[i][posMl] & ~maskSl) | (pgm_read_byte(pFont+fontM)>>(8-pgm_read_byte(pFont+posPf)));
        }
      }
      else {
        printCharShadow(sym, pos, GREEN);
        printCharShadow(sym, pos, RED);
      }
    }
  }
}

// печатем массив
void MaTrix::printArray() {
	Serial.println("Array");
  for(byte color=RED; color<YELLOW; color++){
    Serial.println((color)?"GREEN":"RED");
    for(int i=0; i<8; i++){
      for(int j=color; j<8; j=j+2) {
        for(int k=7; k>=0; k--){
          //Serial.print(bitRead(array[i][j], k), DEC);
          Serial.print((bitRead(array[i][j], k))?char(174):char(32));
        }
        //Serial.print(" ");
      }
      Serial.println();
    }
    Serial.println();
  }
}
// печатем массив
void MaTrix::printShadow() {
	Serial.println("Shadow");
  for(byte color=RED; color<YELLOW; color++){
    Serial.println((color)?"GREEN":"RED");
    for(int i=0; i<8; i++){
      for(int j=color; j<8; j=j+2) {
        for(int k=7; k>=0; k--){
          //Serial.print(bitRead(array[i][j], k), DEC);
          Serial.print((bitRead(shadow[i][j], k))?char(174):char(32));
        }
        //Serial.print(" ");
      }
      Serial.println();
    }
    Serial.println();
  }
}

void MaTrix::clearLed() {
  for(int i=0; i<8; i++) for(int j=0; j<8; j++) array[i][j]=0;
}

void MaTrix::clearShadow() {
  for(int i=0; i<8; i++) for(int j=0; j<8; j++) shadow[i][j]=0;
}

//void display() 
// отображение дисплея
ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  if (row++ == 8) row = 0;
  PORTL&=~(1<<7); // установили 7 бит в 0
  PORTL|=(1<<6); // установили 6 бит в 1 (погасили строку)
  // передаем последовательно данные
  // если матрица "боком" (0,0 - справа снизу) SPI.setBitOrder(MSBFIRST);
  for(char i=0; i < 8; i++) SPI.transfer(~getByte(row,i));
  // если матрица "прямо" (0,0 - слева снизу) SPI.setBitOrder(LSBFIRST);
  //for(char i=0; i < 8; i++) SPI.transfer(~array[7-row][i]);
  PORTL = (PORTL & ~addrmask) | (row) | (1<<7);
  // яроксть регулируется ШИМ на 5 пине (вынесено из функции обработки прерывания)
  PORTL&=~(1<<6); // установили 6 бит в 1 (зажгли строку)
}


byte getByte(byte row, byte col) {
  byte mask=1;
  if (row>0) mask<<=row;
  return 
    (array[0][col]&mask?B10000000:0)|
    (array[1][col]&mask?B1000000:0)|
    (array[2][col]&mask?B100000:0)|
    (array[3][col]&mask?B10000:0)|
    (array[4][col]&mask?B1000:0)|
    (array[5][col]&mask?B100:0)|
    (array[6][col]&mask?B10:0)|
    (array[7][col]&mask?1:0);
}

/*
byte getByte(byte row, byte col){
  byte mask=1;
  if (row>0) mask<<=row;
  return 
    (array[7][col]&mask?B10000000:0)|
    (array[6][col]&mask?B1000000:0)|
    (array[5][col]&mask?B100000:0)|
    (array[4][col]&mask?B10000:0)|
    (array[3][col]&mask?B1000:0)|
    (array[2][col]&mask?B100:0)|
    (array[1][col]&mask?B10:0)|
    (array[0][col]&mask?1:0);
}
*/
void MaTrix::setFont(unsigned char *Font){
  pFont = Font;
}


// печатаем строку
void MaTrix::printString(String s, byte pos, byte color, unsigned char *Font, char effect, int speed) {
  pFont = Font;
  int i, j, k;
  unsigned long ready;
  if (effect==0) {	// без эффекта
	unsigned char buf[s.length()+1];
	for (unsigned char i=0; i<s.length(); i++) {
		buf[i]=s[i];
	}
	buf[s.length()]='\0';  
	printStr(buf, pos, color);
  } 
  else {
  clearShadow();
  printStringShadow(s, pos, color);
  if (effect==1) {	// сдвижка вверх
	  for (i=0; i<8; i++){
		// сдвигаем массив array на 1 строку вверх
		for(j=0; j<8; j++){
		  for(k=0; k<8; k++) {
			array[k][j]=array[k+1][j];
		  }
		  // достаем из массива shadow очередную строку и размещаем ее в нижнюю
		  array[7][j]=shadow[i][j];
		}
		ready=millis()+speed;
		while(millis()<ready) code();
	  }
  }
  else if (effect==2) {	// сдвижка вниз 
	  for (i=0; i<8; i++){
		// сдвигаем массив array на 1 строку вниз
		for(j=0; j<8; j++){
		  for(k=7; k>=0; k--) {
			array[k][j]=array[k-1][j];
		  }
		  // достаем из массива shadow очередную строку и размещаем ее в верхнюю
		  array[0][j]=shadow[7-i][j];
		}
		ready=millis()+speed;
		while(millis()<ready) code();
	  }
	}
	else if (effect==3) {	// сдвиг влево 
	
	byte empty;
	
	if(longStringFlag == true){
	//int nChar=int(32/pgm_read_byte(pFont));
		empty=32-pgm_read_byte(pFont)*(int)(32/pgm_read_byte(pFont));
		//Serial.println(empty);
		for(k=0; k<empty; k++) {
			for(j=0; j<8; j++) {
				for(i=0; i<8; i++){
					shadow[i][j]=shadow[i][j]<<1;
					if(j<6){
						bitWrite(shadow[i][j],0,bitRead(shadow[i][j+2],7));
					}
				}
			}
		}
	}
	
	//printArray();
	//printShadow();
		for(k=0; k<(32-empty*longStringFlag); k++){	// проходим 32 шага - чтобы самый крайний правый пиксель уехал влево до конца
			for(j=0; j<8; j++){		// перебираем столбцы (начинаем с самого левого)
				for(i=0; i<8; i++){	// строки (с нулевой)
					array[i][j]=array[i][j]<<1;	// сдвигаем байты основного массива на 1 (старший бит - теряется, младший - 0)
					
					// заполняем младший бит текущего байта
					if(j<6){	// если требуемый байт в рамках основного массива
						// берем старший бит из предыдущего байта нужного цвета
						bitWrite(array[i][j],0,bitRead(array[i][j+2],7));
					}
					else {		// если в текущем массиве нет нужного байта (за границей массива)
								// берем из начала теневого массива (подтягиваем теневой массив)
						if(j==6){	// "красный" байт
							bitWrite(array[i][6],0,bitRead(shadow[i][0],7));
						}
						else {		// "зеленый" байт
							bitWrite(array[i][7],0,bitRead(shadow[i][1],7));
						}
					}
					
					// теневой массив тоже сдвигаем
					if(k>0){
						shadow[i][j]=shadow[i][j]<<1;
						if(j<6){
							bitWrite(shadow[i][j],0,bitRead(shadow[i][j+2],7));
						}
					}
				}
			}
			
			ready=millis()+speed;
			while(millis()<ready) code();
		}
	//Serial.println("----------------------------------------");
	//printArray();
	//printShadow();
	}
  }
}

void MaTrix::printRunningString(String s, byte color, unsigned char *Font, int speed) {
	pFont = Font;
	longStringFlag = true;
	//clearLed();
	clearShadow();
	int i = 0;
	int nChar=int(32/pgm_read_byte(pFont));
	String sp;
	int iterations = int((s.length())/nChar);
	for(i=0; i<=iterations; i=i++) {
		sp=s.substring(i*nChar, nChar+i*nChar);
		printString(sp, nChar-1, color, Font, 3, speed);
	}
	longStringFlag = false;
	// чтобы не оставались "хвосты"
	printString(" ", nChar-1, color, Font, 3, speed);	
}
		
  
// печатаем массив символов
void MaTrix::printStr(unsigned char *s, byte pos, byte color) {
  byte p = pos;
  while (*s) 
    {
      printChar(*s, p, color);
      s++;
      p--;
    }
}
  // печатаем строку
void MaTrix::printStringShadow(String s, byte pos, byte color) {
  unsigned char buf[s.length()+1];
  for (unsigned int i=0; i<s.length(); i++) {
    buf[i]=s[i];
  }
  buf[s.length()]='\0';  
  printStrShadow(buf, pos, color);
}
  
// печатаем массив символов
void MaTrix::printStrShadow(unsigned char *s, byte pos, byte color) {
  byte p = pos;
  while (*s) 
    {
      printCharShadow(*s, p, color);
      s++;
      p--;
    }
}
  