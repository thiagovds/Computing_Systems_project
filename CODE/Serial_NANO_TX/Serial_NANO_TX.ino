#define LED D0    

void setup() {
  pinMode(LED, OUTPUT);      // set LED pin as output
  digitalWrite(LED, LOW);    // switch off LED pin

  Serial.begin(9600);               // initialize serial communication at 9600 bits per second:
  Serial1.begin(9600);            // initialize UART with baud rate of 9600
}
void loop() {
  if (Serial.read() == '1'){
    Serial1.println('1');
    digitalWrite(LED, HIGH);
    Serial.println("LEDS ON");
  }
  else if (Serial.read() == '0'){
    Serial1.println('0');
    digitalWrite(LED, LOW);
    Serial.print("LEDS OFF");
  }
}
