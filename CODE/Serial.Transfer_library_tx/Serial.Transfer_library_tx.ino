#include "SerialTransfer.h"

SerialTransfer myTransfer;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);
}

void loop()
{
  float myFloat = 5.2;
  myTransfer.txObj(myFloat, sizeof(myFloat));
  myTransfer.sendData(sizeof(myFloat));
  delay(100);
}
