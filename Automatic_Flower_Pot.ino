//Define
//aa
#define MOISTURE_SIGNAL A0

#define SONIC_TRIG A1
#define SONIC_ECHO A2

#define LCD_RS 12
#define LCD_EN 6
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

#define PIEZO_SIGNAL 13

#define MOTOR_SIGNAL 8 //x

#define LED_SIGNAL 7 //x

#define WATER_LEVEL 20

#define DATAQU 15 // sonic data quantity

#define MAXLUX 2000
#define MINLUX 1000

//End Define

//Include 

#include <LiquidCrystal.h>
#include "BH1750.h" // Sensor Library
#include <Wire.h> // I2C Library
#include "Adafruit_NeoPixel.h"

//End include headers


//Global variables
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
BH1750 lightMeter;
Adafruit_NeoPixel ledbar = Adafruit_NeoPixel(10, LED_SIGNAL, NEO_GRB + NEO_KHZ800); //(led갯수, 핀번호, 그냥 냅두면되는거)
//End Global Variables

void setup() 
{
  Serial.begin(9600);

  //빛센서
  lightMeter.begin();

  //초음파
  pinMode(SONIC_TRIG, OUTPUT);
  pinMode(SONIC_ECHO, INPUT);

  //LCD
  lcd.begin(2, 16); //col, row
  
  //LED
  ledbar.begin();
  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, LOW);
} //End setup


void loop() 
{
  int distance = 0;
  float moisture = 0;
  uint16_t lux = 0;
  int value = 0;
  

  getFilteredDistance(&distance);
  activateMotor(&moisture);
  getBrightness(&lux);
  checkLevel(&distance);

  value = setLEDlux(&lux);
  for(int i = 0; i < 10; i++)
    ledbar.setPixelColor(i, value, value, value); //(i번째 led, r, g, b)  rgb순서는 모르겟
  ledbar.show();
  
  showLCD(distance, moisture, lux);
  
}//End loop

void activateMotor(float * moisture)
{
  getMoisture(moisture);
  
  if((int)*moisture < 75)
  {
    digitalWrite(MOTOR_SIGNAL, HIGH);
    delay(1000); //나중에 5000으로 변경
    digitalWrite(MOTOR_SIGNAL, LOW);
  }
}

void showLCD(int distance, float moisture, uint16_t lux)
{
  /*lcd.clear();
  lcd.write("Distance : ");
  lcd.print(distance);
  lcd.write("cm");*/
  lcd.write("Lux : ");
  lcd.print(lux);
  lcd.write(" lx");
  lcd.setCursor(0, 1);
  lcd.write("Moisture : ");
  lcd.print((int)moisture);
  lcd.write("%");
  

}

void getFilteredDistance(int * distance)
{
    int readData[DATAQU] = {0};
    int count = 0;
    double avg = 0;
    double dev = 0;
    double realvalue = 0;

    int d1 = 0;
    int d2 = 0;

    for(int i = 0; i < DATAQU; i++)
        readData[i] = getDistance();

    for(int i = 0; i < DATAQU; i++)
    {
        d1 += readData[i];
        d2 += readData[i] * readData[i];

    }
    avg = d1/DATAQU;

    dev = sqrt((d2/DATAQU) - (d1/DATAQU) * (d1/DATAQU)); //제곱의평균 - 평균의 제곱

    for(int i = 0; i < DATAQU; i++)
    {
        if(readData[i] > avg + dev || readData[i] < avg - dev)
        {
            readData[i] = 0;
            count += 1;
        }
    }

    d1 = 0;
    for(int i = 0; i < DATAQU; i++)
        d1 += readData[i];

    realvalue = (double)d1 / (DATAQU - count);

    Serial.print("filtered distance : ");
    Serial.println(realvalue);
    *distance = (double)realvalue;
}

int getDistance(void)
{
  long duration = 0;
  int distance = 0;
  
  digitalWrite(SONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONIC_TRIG, LOW);

  duration = pulseIn(SONIC_ECHO, HIGH); //물체에 반사되어돌아온 초음파의 시간을 변수에 저장합니다.
  distance = duration * 17 / 1000;
  //Serial.print("\nDIstance : ");
  //Serial.print(distance); //측정된 물체로부터 거리값(cm값)을 보여줍니다.
  //Serial.println(" Cm");
  
  return distance;
  

}

void getMoisture(float * moisture)
{
  *moisture = (float)analogRead(MOISTURE_SIGNAL) / 820 * 100;
}

void getBrightness(uint16_t * lux)
{
  *lux = lightMeter.readLightLevel();
  Serial.print("lux ");
  Serial.println(*lux);
}

void checkLevel(int * distance)
{
  if(*distance < WATER_LEVEL)
    tone(PIEZO_SIGNAL, 2500);
  else
    noTone(PIEZO_SIGNAL);
}

int setLEDlux(int * lux)
{
  int remainderlux = (MAXLUX - MINLUX);

  getBrightness(lux);
  
  if(*lux <= MINLUX)
    return 255;
  else if(*lux > MAXLUX)
    return 0;
  else
    return (int)(255 - ((double)*lux - MINLUX) / remainderlux * 255);
}

