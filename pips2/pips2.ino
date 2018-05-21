#include <SoftwareSerial.h>
#include <MicroNMEA.h>

#define PIP_EVERY_MINUTE 0
#define PRINT_TIME 0

#define LED_PIN 13
#define AUDIO_OUTPUT_PIN 6
#define PPS_PIN 2
#define RX_PIN 3
#define TX_PIN 4

int syncCount = 0;
uint8_t hour, minute, second;
bool synced = false;
char nmeaBuffer[85];

SoftwareSerial gpsSerial(RX_PIN, TX_PIN); // RX, TX
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void setup() {
  Serial.begin(9600);

  Serial.println("Pip generator, see https://github.com/M0LTE/pips");
  
  pinMode(LED_PIN,OUTPUT); // fix LED
  
  pinMode(AUDIO_OUTPUT_PIN, OUTPUT); // audio

  pinMode(PPS_PIN, INPUT_PULLUP); // pps
}

void pps(void) {

  // every minute at an innocuous point in the minute, ensure GPS time matches internal time
  if (second == 31) {
    synced = false;
  }

#ifdef PRINT_TIME
  Serial.print(hour);
  Serial.print(":");  
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
#endif

  if (synced) {
    if (second == 0 && (PIP_EVERY_MINUTE || minute == 0 || minute == 15 || minute == 30 || minute == 45  )) {
      // long pip - 500ms @ 1kHz
      tone(AUDIO_OUTPUT_PIN, 1000, 500);
    } else if (second >= 55 && (PIP_EVERY_MINUTE || minute == 59 || minute == 14 || minute == 29 || minute == 44)) {
      // short pip - 100ms @ 1kHz
      tone(AUDIO_OUTPUT_PIN, 1000, 100);
    }

    // increment the "clock"
    second++;

    // roll over seconds, minutes and hours
    if (second == 60) {
      minute++;
      second = 0;
    }
  
    if (minute == 60) {
      hour++;
      minute = 0;
    }
  
    if (hour == 24) {
      hour = 0;
    }
  }
}

void serialFlush(){
  while(gpsSerial.available() > 0) {
    char t = gpsSerial.read();
  }
}

void loop() {
  if (!synced) {
    
    // software serial interferes with tone generation, so turn off the interrupt
    detachInterrupt(digitalPinToInterrupt(PPS_PIN));
    gpsSerial.begin(9600);

    // empty out old sentences from the buffer
    serialFlush();
    while (!synced){
      while (gpsSerial.available()) {
        char c = gpsSerial.read();
        if (nmea.process(c)) {
          if (nmea.isValid()) {

            // NMEA sentences come in batches, need a full set of sentences from the GPS to
            // ensure at least one of them is a sentence containing the time.
            if (syncCount == 20){
              hour = nmea.getHour();
              minute = nmea.getMinute();
              second = nmea.getSecond();
              synced = true;
              syncCount = 0;
            } else {
              syncCount++;
            }
          } else {
            digitalWrite(LED_PIN, LOW);
          }
        }
      }
    }
    
    gpsSerial.end();
    digitalWrite(LED_PIN, HIGH);
    attachInterrupt(digitalPinToInterrupt(PPS_PIN), pps, RISING);
  }
}
