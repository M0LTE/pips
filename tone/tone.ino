#include <SoftwareSerial.h>
#include <MicroNMEA.h>

SoftwareSerial gpsSerial(3, 4); // RX, TX
char nmeaBuffer[85];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void setup() {
  Serial.begin(9600);
  Serial.println("Waiting for GPS");
  gpsSerial.begin(9600);
  pinMode(13,OUTPUT);
  pinMode(6,OUTPUT);

  //pinMode(2, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(2), pps, RISING);
}

void pps(void) {
}

void loop() {
  // put your main code here, to run repeatedly:
  gpsSerial.end();
  tone(6,1000,100);
  delay(1000);
  gpsSerial.begin(9600);
}
