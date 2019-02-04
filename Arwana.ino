#include <Adafruit_GFX.h>
#include <UTFTGLUE.h>
#include <SPI.h>             // f.k. for Arduino-1.5.2

UTFTGLUE myGLCD(0x9488, A2, A1, A3, A4, A0);
//UTFTGLUE myGLCD(0x9488,A2,A1,A3,A6,A0);
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

#include <MCUFRIEND_kbv.h>
#if defined(_GFXFONT_H_)           //are we using the new library?
#include <Fonts/FreeSans9pt7b.h>
#define ADJ_BASELINE 11            //new fonts setCursor to bottom of letter
#else
#define ADJ_BASELINE 0             //legacy setCursor to top of letter
#endif
MCUFRIEND_kbv tft;
uint16_t ID;

#define YELLOW  0xFFE0
#define BLACK   0x0000
#define WHITE   0xFFFF

uint32_t runTime = -99999;       // time for next update
/*------------------------------------------------------------------------------------------*/
/* GPS Initialization */
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin = 19, TXPin = 18;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ssGPS(RXPin, TXPin);
bool pertama = true, outBorder = false, outBorder2 = false;
unsigned long previousMillis = 0;        // will store last time LED was updated
double previousLatitude = 0, previousLongitude = 0;

  static const double FISH_LON = 110.394989, FISH_LAT = -7.762480; 
/*------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------*/
/* Compass Initialization */
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#include "Wire.h"

// I2Cdev and HMC5883L must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "HMC5883L.h"

// class default I2C address is 0x1E
// specific I2C addresses may be passed as a parameter here
// this device only supports one I2C address (0x1E)
HMC5883L mag;

int16_t mx, my, mz;

#define LED_PIN 13
bool blinkState = false;

float uncalibrated_values[3],calibrated_values[3];

/*------------------------------------------------------------------------------------------*/

// Declare which fonts we will be using
/*#if !defined(SevenSegNumFont)
extern uint8_t SevenSegNumFont[];    //.kbv GLUE defines as GFXFont ref
#endif
*/

const int x_offset = 30;
const int y_offset = 128;
const int z_offset = 0;
int last_dx, last_dy, dx, dy, dx_az, dy_az, last_dx_az, last_dy_az;

const int compassX  = 60;
const int compassY  = 60;
const int radius = 55;

void setup() {

  Serial.begin(115200);
//  ssGPS.begin(GPSBaud);
  Serial1.begin(9600);
  randomSeed(analogRead(5));   //.kbv Due does not like A0
  pinMode(A0, OUTPUT);       //.kbv mcufriend have RD on A0
  digitalWrite(A0, HIGH);

  Wire.begin();
  Serial.println("Initializing I2C devices...");
  mag.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(mag.testConnection() ? "HMC5883L connection successful" : "HMC5883L connection failed");
  myGLCD.clrScr();

  tft.reset();
  ID = tft.readID();
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  tft.begin(ID);
  tft.setRotation(1);
#if defined(_GFXFONT_H_)
  tft.setFont(&FreeSans9pt7b);
#endif

  // Setup the LCD
  myGLCD.InitLCD();
  //  myGLCD.setFont(SmallFont);
 // myGLCD.setFont(BigFont);

  myGLCD.clrScr();

  myGLCD.fillScr(0, 0, 0);
  
//  myGLCD.setColor(0, 0, 255);
//  myGLCD.fillRoundRect(155, 55, 319, 120);

  //myGLCD.setBackColor(255, 0, 0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRect(0, 0, 319, 234);
  myGLCD.print(" Arwana, Fishing Ground 2017 ", CENTER,229);

  last_dx = compassX;
  last_dy = compassY;
  last_dx_az = compassX;
  last_dy_az = compassY;

}

void loop() {
  
  double angle, angle_az;
  int x, y, z;
  if (millis() - runTime >= 500) { // Execute every 500ms
  runTime = millis();
  
  displayInfoGPS();
  distance();
  velocityGPS();
  double az = azhimuth();
  Serial.print("az: ");
  Serial.print(az);

/*------------------------------------------------------------------------------------------*/
    mag.getHeading(&mx, &my, &mz);

    uncalibrated_values[0] = mx;
    uncalibrated_values[1] = my;
    uncalibrated_values[2] = mz;
  
    transformation(uncalibrated_values);

    // display tab-separated gyro x/y/z values
    Serial.print("mag:\t");
//    Serial.print(calibrated_values[0]); Serial.print("\t");
//    Serial.print(calibrated_values[1]); Serial.print("\t");
    Serial.print(calibrated_values[2]); Serial.println("\t");
    
// To calculate heading in degrees. 0 degree indicates North
    float heading = atan2(my, mx);
    if(heading < 0)
      heading += 2 * M_PI;
    Serial.print("heading:\t");
    Serial.println(heading * 180/M_PI);
/*------------------------------------------------------------------------------------------*/

    
  angle = heading* 180/M_PI;

  drawCompass();
  //  angle= atan2((double)y + y_offset,(double)x + x_offset)* (180 / 3.141592654) + 180;
  dx = (0.9 * radius * cos((angle - 90) * 3.14 / 180)) + compassX; // calculate X position
  dy = (0.9 * radius * sin((angle - 90) * 3.14 / 180)) + compassY; // calculate Y position
  dx_az = (0.9 * radius * cos((az - 90) * 3.14 / 180)) + compassX; // calculate X position
  dy_az = (0.9 * radius * sin((az - 90) * 3.14 / 180)) + compassY; // calculate Y position

  myGLCD.setColor(0, 0, 0);
  arrow(last_dx, last_dy, compassX, compassY, 10, 10, 255, 255, 0); // Erase last arrow
  arrow(dx, dy, compassX, compassY, 10, 10, 255, 255, 0);            // Draw arrow in new position
  arrow(last_dx_az, last_dy_az, compassX, compassY, 10, 10, 255, 255, 255); // Erase last arrow
  arrow(dx_az, dy_az, compassX, compassY, 10, 10, 255, 255, 255);            // Draw arrow in new position
  last_dx = dx;
  last_dy = dy;
  last_dx_az = dx_az;
  last_dy_az = dy_az;

  tft.setCursor(140,100 + ADJ_BASELINE);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW,BLACK);
  tft.print("Kapal : ");
  tft.print(angle);

  /*double azhimut;
  myGLCD.print("Azhimut : ", 10, (compassY + 80));
  myGLCD.printNumI(azhimut, 60 , (compassY + 80));
  double rotate = 180;
  myGLCD.print("Rotate : ", 10, (compassY + 90));
  myGLCD.printNumI(abs(azhimut - angle), 60 , (compassY + 90));
  */
  myGLCD.setColor(255, 255, 0);




//  delay(25);
  }  
}
