#include <SPI.h>
#include <SD.h>
unsigned long t = 0;


void setup() {
  Serial.begin(115200);
  pinMode(0, INPUT_PULLUP);

  while(digitalRead(0)){}
  randomSeed(millis());
  for(int i = 0; i < 15; i++){
    t+=random(1000,3000);
    Serial.print(t);
    Serial.print(",");
    Serial.println(random(0,180));
  }
}

void loop() {
  
}
