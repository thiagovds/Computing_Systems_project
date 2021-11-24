#include <ESP8266WiFi.h>

#define SIGNAL_FROM_ARDUINO 16   //PIN D0!!!!! TO INTERCONNECT 2 BOARDS
int a = 0;

const char* ssid = "Thai";   // Enter the namme of your WiFi Network.

const char* password = "NotYourWifi33";  // Enter the Password of your WiFi Network.

char server[] = "mail.smtp2go.com";   // The SMTP Server

 

WiFiClient espClient;

void setup()

{
  pinMode(SIGNAL_FROM_ARDUINO, INPUT);

  Serial.begin(9600);    //original code had 115200 baud rate!

  delay(10);

  Serial.println("");

  Serial.println("");

  Serial.print("Connecting To: ");

  Serial.println(ssid);

  WiFi.begin(ssid, password);

 

  while (WiFi.status() != WL_CONNECTED)

  {

    delay(500);

    Serial.print("*");

  }

  Serial.println("");

  Serial.println("WiFi Connected.");

  Serial.print("IP address: ");

  Serial.println(WiFi.localIP());

  //byte ret = sendEmail();                       //-------------------------------

 }

 

void loop()

{
  a = digitalRead(SIGNAL_FROM_ARDUINO);
  if(a == HIGH)
  {
      byte ret = sendEmail();
      Serial.println(a);
      Serial.println("Email Sent!");
      delay(10000);
  }
  //       ELSE ENTER lowpower mode!!!               TO BE IMPLEMENTED!!!

}

 

byte sendEmail()

{

  if (espClient.connect(server, 2525) == 1)

  {

    Serial.println(F("connected"));

  }

  else

  {

    Serial.println(F("connection failed"));

    return 0;

  }

  if (!emailResp())

    return 0;

 

  Serial.println(F("Sending EHLO"));

  espClient.println("EHLO www.example.com");

  if (!emailResp())

    return 0;

 

  Serial.println(F("Sending auth login"));

  espClient.println("AUTH LOGIN");

  if (!emailResp())

    return 0;

   

  Serial.println(F("Sending User"));

 

 

  espClient.println("YXJkdWlubzMzQHByb3Rvbm1haWwuY29t"); // Your encoded Username

  if (!emailResp())

    return 0;

 

  Serial.println(F("Sending Password"));

 

  espClient.println("YiYhTUZxNDJeIW03SyZN");// Your encoded Password

  if (!emailResp())

    return 0;

 

  Serial.println(F("Sending From"));

 

  espClient.println(F("MAIL From: arduino33@protonmail.com")); // Enter Sender Mail Id

  if (!emailResp())

    return 0;

 

  Serial.println(F("Sending To"));

  espClient.println(F("RCPT To: thiagovds7@hotmail.com")); // Enter Receiver Mail Id

  if (!emailResp())

    return 0;

 

  Serial.println(F("Sending DATA"));

  espClient.println(F("DATA"));

  if (!emailResp())

    return 0;

  Serial.println(F("Sending email"));

 

  espClient.println(F("To:  thiagovds7@hotmail.com")); // Enter Receiver Mail Id

  // change to your address

  espClient.println(F("From: arduino33@protonmail.com")); // Enter Sender Mail Id

  espClient.println(F("Subject: PILL DISPENSER ERROR!\r\n"));

  espClient.println(F("Dear Caregiver,\n"));  /*  THIS IS WHERE WE ADD THE EMAIL  */ 

  espClient.println(F("This is an automatic email, the RAPID pill dispenser has run into a dispensing error!"));

  espClient.println(F("Please contact patient immediately."));

  //

  espClient.println(F("."));

  if (!emailResp())

    return 0;

  //

  Serial.println(F("Sending QUIT"));

  espClient.println(F("QUIT"));

  if (!emailResp())

    return 0;

  //

  espClient.stop();

  Serial.println(F("disconnected"));

  return 1;

}

 

byte emailResp()

{

  byte responseCode;

  byte readByte;

  int loopCount = 0;

 

  while (!espClient.available())

  {

    delay(1);

    loopCount++;

  

    if (loopCount > 20000)

    {

      espClient.stop();

      Serial.println(F("\r\nTimeout"));

      return 0;

    }

  }

 

  responseCode = espClient.peek();

  while (espClient.available())

  {

    readByte = espClient.read();

    Serial.write(readByte);

  }

 

  if (responseCode >= '4')

  {

   

    return 0;

  }

  return 1;

}
