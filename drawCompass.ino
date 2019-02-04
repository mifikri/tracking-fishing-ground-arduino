void drawCompass()
{
  int dxo, dyo, dxi, dyi;
  myGLCD.setColor(0, 255, 0);
  myGLCD.drawCircle(compassX, compassY, radius);
  myGLCD.setColor(0, 255, 0);
  for (float i = 0; i < 360; i = i + 22.5) {
    dxo = radius * cos((i - 90) * 3.14 / 180);
    dyo = radius * sin((i - 90) * 3.14 / 180);
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    myGLCD.drawLine(dxo + compassX, dyo + compassY, dxi + compassX, dyi + compassY);
  }
  myGLCD.setColor(255, 0, 0);
  myGLCD.print("W", (compassX - 45), (compassY - 5));
  myGLCD.print("N", (compassX - 3), (compassY - 45));
  myGLCD.print("E", (compassX + 45), (compassY - 5));
  myGLCD.print("S", (compassX - 3), (compassY + 40));
  
}

void arrow(int x2, int y2, int x1, int y1, int alength, int awidth, int b, int g, int r) {
  float distance;
  int dx, dy, x2o, y2o, x3, y3, x4, y4, k;
  distance = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
  dx = x2 + (x1 - x2) * alength / distance;
  dy = y2 + (y1 - y2) * alength / distance;
  k = awidth / alength;
  x2o = x2 - dx;
  y2o = dy - y2;
  x3 = y2o * k + dx;
  y3 = x2o * k + dy;
  //
  x4 = dx - y2o * k;
  y4 = dy - x2o * k;
  myGLCD.drawLine(x1, y1, x2, y2);
//  myGLCD.drawLine(x1, y1, dx, dy); //no
//  myGLCD.drawLine(x3, y3, x4, y4);
//  myGLCD.drawLine(x3, y3, x2, y2);
//  myGLCD.drawLine(x2, y2, x4, y4);
  myGLCD.setColor(g, b, r);
}

void transformation(float uncalibrated_values[3])    
{
  //calibration_matrix[3][3] is the transformation matrix
  //replace M11, M12,..,M33 with your transformation matrix data
  double calibration_matrix[3][3] = 
  {
    {6.052,-0.164,-0.261}, // dicari di app magmaster nilai-nilainya
    {0.152,2.686,-0.077},
    {-4.878,-0.08,3.373}  
  };
  //bias[3] is the bias
  //replace Bx, By, Bz with your bias data
  double bias[3] = 
  {
    107.694,   //dicari di app MagMaster Biasnya
    2.566,
   -234.288
  };  
  //calculation
  for (int i=0; i<3; ++i) uncalibrated_values[i] = uncalibrated_values[i] - bias[i];
  float result[3] = {0, 0, 0};
  for (int i=0; i<3; ++i)
    for (int j=0; j<3; ++j)
      result[i] += calibration_matrix[i][j] * uncalibrated_values[j];
  for (int i=0; i<3; ++i) calibrated_values[i] = result[i];
}

