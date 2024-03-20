#include <SPI.h>
#include <SD.h>
#include "Scenario.h"
#include "servo_control.h"

#define SERVOS 3  //Liczba serw fizycznie możliwych do podłączenia

//bool servoEnable[SERVOS] = {1, 1, 1};
const String filePrefix = "Servo";  //Prefiks pliku scenariusza

int servoIndex[SERVOS];  //Numer serwomechanizmu z nazwy pliku
int fileSize[SERVOS];    //Liczba linijek scenariusza
int numOfFiles = 0;      //Liczba plików scenariusza na karcie SD

bool allFilesEnded = 1;  //Flaga czy wszystkie pliki się skończyły; 1 - skończył się, 0 - nie

unsigned long startTime;

void setup() {
  Serial.begin(115200);
  pinMode(0, INPUT_PULLUP);
  while (digitalRead(0)) {}     // CZEKA NA PRZYCISK ID0
  delay(2000);                  //roboczo

  getFilesData(filePrefix, numOfFiles, fileSize, servoIndex);

  Scenario** scenarios = new Scenario*[numOfFiles];
  for (int scenarioIndex = 0; scenarioIndex < numOfFiles; scenarioIndex++) {
    scenarios[scenarioIndex] = new Scenario(fileSize[scenarioIndex]);
  }

  copyFiles(scenarios, filePrefix, numOfFiles, fileSize, servoIndex);

  setStartPosition(scenarios, numOfFiles);

  Serial.println("\nSTART SCENARIO?\n");
  while (digitalRead(0)) {}  // CZEKA NA PRZYCISK ID0
  delay(500);                //roboczo

  startScenarios(scenarios, numOfFiles, startTime, allFilesEnded);

  while (handleScenarios(scenarios, numOfFiles, startTime, allFilesEnded))
    ;

  Serial.println("\nEND OF ALL FILES.");

  delete[] scenarios;
  Serial.println("Deleted dynamic arrays");  //roboczo
}

void loop() {
}
