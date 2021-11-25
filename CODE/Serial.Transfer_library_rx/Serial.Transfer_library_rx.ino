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
  if(myTransfer.available())
  {
    float myFloat;
    
    Serial.print("New Data: ");
    myTransfer.rxObj(myFloat, sizeof(myFloat));
    Serial.println(myFloat);
  }
  else if(myTransfer.status < 0)
  {
    Serial.print("ERROR: ");
    Serial.println(myTransfer.status);
  }
}
