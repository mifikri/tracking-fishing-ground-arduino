static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial1.available())
      gps.encode(Serial1.read());

  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

void displayInfoGPS()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
      Serial.print(gps.location.lat(), 6);
      Serial.print(F(","));
      Serial.print(gps.location.lng(), 6);
      myGLCD.print("Latitude ", 10, 200);
      tft.setCursor(10,210);
      tft.setTextSize(1);
      tft.setTextColor(YELLOW, BLACK);
      tft.print(gps.location.lat(),6);

      myGLCD.print(" Longitude", 70, 200);
      tft.setCursor(70,210);
      tft.setTextSize(1);
      tft.setTextColor(YELLOW, BLACK);
      tft.print(gps.location.lng(),6);

      
      if (pertama) {
          pertama = false;
          previousLatitude = gps.location.lat();
          previousLongitude = gps.location.lng();
      }

  /*
        Serial.print(" state polygon inside =  ");
        Serial.print(pointInPolygon( gps.location.lng(), gps.location.lat()));
        if(pointInPolygon( gps.location.lng(), gps.location.lat()) == false) //kalo didalem = true, kalo diluar = false
        {
          lcd.setCursor(0,0);
          lcd.print(gps.location.lat(), 6);
          lcd.setCursor(0, 1);
          lcd.print(gps.location.lng(), 6);
          lcd.setCursor(12, 1);
          lcd.print("-_-");
          Serial.print("  SUDAH LEWAT  ");
          outBorder = true;
        }

        else
        {
          lcd.setCursor(0, 1);
          lcd.print(gps.location.lng(), 6);
          lcd.setCursor(11, 1);
          lcd.print("BELUM");
          Serial.print("BELUM LEWAT");
          outBorder = false;
        }

*/
  }
  else
  {
    Serial.print(F("INVALID"));

  }
  printDateTime(gps.date, gps.time);

  Serial.println();
  if (millis() > 5000 && gps.charsProcessed() < 10)
  Serial.println(F("No GPS data received: check wiring"));

}

void velocityGPS()
{
  if (gps.location.isValid())
  {
  Serial.print("kecepatan : ");
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  tft.setCursor(140,120 + ADJ_BASELINE);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW, BLACK);
  tft.print("Kec : ");
  tft.print(gps.speed.kmph());
  tft.setCursor(270,120 + ADJ_BASELINE);
  tft.print("KM/h");
  }
}

float distance()
{
  if (gps.location.isValid())
  {
  float distanceKmToFish = distanceKM(gps.location.lat(), gps.location.lng(), FISH_LAT, FISH_LON) / 1000;

  printInt(distanceKmToFish, gps.location.isValid(), 3);
  tft.setCursor(140,60 + ADJ_BASELINE);
  tft.setTextColor(YELLOW, BLACK);
  tft.setTextSize(2);
  tft.print("Jarak:");
  tft.print(distanceKmToFish, 2);
  tft.setCursor(290,60 + ADJ_BASELINE);
  tft.setTextColor(YELLOW, BLACK);
  tft.print("KM");
  }
}

double azhimuth(){
  double courseToFish =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      FISH_LAT, 
      FISH_LON);

  printFloat(courseToFish, gps.location.isValid(), 7, 2);
  tft.setCursor(140,80 + ADJ_BASELINE);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print("Arah: ");
  tft.print(courseToFish);
  return courseToFish;

}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
    myGLCD.printNumI(d.day(), 145 , 200);
    myGLCD.printNumI(d.month(), 165 , 200);
    myGLCD.printNumI(d.year(), 185 , 200);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
      if (gps.time.isValid())
  {
    char sz[32];
      if (t.hour() + 7 < 10) 
      {
        Serial.print(F("0"));
        Serial.print(t.hour() + 7);
        myGLCD.printNumI(t.hour() + 7, 145 , 210);
      }
      else if (gps.time.hour() + 7 > 23) 
      {
        Serial.print(t.hour() - 17);
        myGLCD.printNumI(t.hour()-17, 145 , 210);
      }
      else 
      {
        Serial.print(gps.time.hour() + 7);
        myGLCD.printNumI(t.hour() + 7, 145 , 210);
      }

      if (gps.time.minute() < 10) 
      {
        Serial.print(F("0"));
      }

      myGLCD.printNumI(t.minute(), 165 , 210);
      if (gps.time.second() < 10) Serial.print(F("0"));
      myGLCD.printNumI(t.second(), 185 , 210);
      Serial.print(F("."));
      if (gps.time.centisecond() < 10) Serial.print(F("0"));
      Serial.print(gps.time.centisecond());  
      
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

float distanceKM(float lat1, float long1, float lat2, float long2)
{
  float delta = radians(long1-long2);
  float sdlong = sin(delta);
  float cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  float slat1 = sin(lat1);
  float clat1 = cos(lat1);
  float slat2 = sin(lat2);
  float clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}
