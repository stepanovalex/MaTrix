#include <SPI.h>
#include <MaTrix.h>
#include <Wire.h>
#include <RTClib.h>
#include <IRremote.h>

// Матрица
MaTrix mymatrix;

extern unsigned char font5x8[];
extern unsigned char font6x8[];
extern unsigned char digit6x8bold[];
extern unsigned char digit6x8future[];
extern byte array[8][8];
extern byte shadow[8][8];

int brightLmax;
int brightLcur;
byte brightL;

// ИК-приемник
int RECV_PIN = 5; // ИК-приемник в Shield MaTrix подключен к 5 цифровому пину
// не забудьте поправить в файле /IRremote/IRremoteInt.h конфигурацию
//   #define IR_USE_TIMER3   // tx = pin 5
// иначе ИК-команды не будут обрабатываться

IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;
int codeType = -1; 
unsigned long codeValue; 
unsigned int rawCodes[RAWBUF]; 
int codeLen; 
int toggle = 0; 

// RTC
RTC_DS1307 RTC;
DateTime now;

// дни недели, месяцы
static char wDay[7][10] =
{
  "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};
static char wMonth[12][4] =
{
  "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

unsigned long ready;
byte color=GREEN;
byte count=0;
byte effect=3;
unsigned int pause;

void setup(){
  Serial.begin(115200);
  
  // RTC
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  // Матрица
  // инициализация 
  mymatrix.init();
  // очистим матрицу
  mymatrix.clearLed();
  
  // ИК-приемник
  irrecv.enableIRIn();
  
  // Пищалка 
  tone(45, 2000, 100); // подключена к 45 пину
}

void loop(){
 if (millis()>ready) {
   now = RTC.now();
   String wD;
   char buff[60];
   char tbuf[6];
   char pbuf[6];
     switch(count) {
         case 0:
           if((now.year()!=2000) || (now.year()!=2165)) {
             sprintf(buff, "%02d%s%02d", now.hour(),(now.second()%2)?":":":",now.minute());
             mymatrix.printString(buff, 4, color, digit6x8future, effect, VFAST);
           }
           ready=millis()+15000;
           while(millis()<ready){
             now = RTC.now();
             code();
             if((now.year()!=2000) || (now.year()!=2165)) sprintf(buff, "%02d%s%02d", now.hour(),(now.second()%2)?":":" ",now.minute());
             mymatrix.printString(buff, 4, color, digit6x8future);
           }
           pause=0;
           
           break;
         case 2:
           sprintf(buff, "%2d %s %4d", now.day(), wMonth[now.month()-1], now.year());
           wD = String(buff);
           code();
           mymatrix.printRunningString(wD, YELLOW, font6x8, FAST);
           pause=0;
           break;
         case 1:
           sprintf(buff, "%s", wDay[now.dayOfWeek()]);
           code();
           wD = "  "+String(buff);
           mymatrix.printRunningString(wD, RED, font6x8, FAST);
           pause=0;
           break;
         default:
           break;
     }
     count++;
     if(count>2) count = 0;
   
   color++;
   if(color>2) {
   color=0;
   effect++;
   if (effect>3) effect=1;
   }
   ready = millis()+pause;
   }

  code();
}

void code(){
  // автоматическая регулировка яркости в зависимости от освещенности
  brightLcur = analogRead(LightSENS);
  if(brightLcur > brightLmax) {
    brightLmax = brightLcur;
  }
  brightL = map(brightLcur, 0, brightLmax, 20, 255);
  mymatrix.brightness(brightL);
  
  // обработка ИК-команд
  if (irrecv.decode(&results)) {
    storeCode(&results);
    irrecv.resume(); 
  }
}
