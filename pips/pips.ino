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

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), pps, RISING);
}

uint8_t hour;
uint8_t minute;
uint8_t second;
bool gpsValid = false;

#define everyMin 0

void pps(void) {
  if (!gpsValid) {
    return;
  }

  Serial.println(second);

  if (second == 54) {
    gpsSerial.end();
    Serial.println("GPS off");
    Serial.println("tick");
    second++;
    return;
  } else if (second >= 55 && second < 60) {
    Serial.println("tick");
  } else if (second == 60) {
    Serial.println("60->0");
    second = 0;
  } else if (second == 1) {
    gpsSerial.begin(9600);
    Serial.println("GPS on");
  }
  
  if (everyMin 
      || (second == 0 && (minute == 0 || minute == 15 || minute == 30 || minute == 45))
      || (second >= 55 && (minute == 59 || minute == 14 || minute == 29 || minute == 44))) {
    if (second == 0) {
      tone(6,1000,500);
      second++;
    } else if (second >= 55) {
      tone(6,1000,100);
      second++;
    }
  }
}

void loop() {

  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (nmea.process(c)) {
      if (nmea.isValid()) {
        if (!gpsValid){
          Serial.println("Got GPS");
          gpsValid = true;
          digitalWrite(13, HIGH);
        }
        
        hour = nmea.getHour();
        minute = nmea.getMinute();
        second = nmea.getSecond() + 1;
        if (second == 60){
          second = 0;
          minute++;
          gpsSerial.begin(9600);
          if (minute == 60){
            minute = 0;
            hour++;
            if (hour == 24){
              hour = 0;
            }
          }
        }
      } else {
        gpsValid = false;
        digitalWrite(13, LOW);
      }
    }
  }
}
