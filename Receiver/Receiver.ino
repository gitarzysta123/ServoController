#include <SPI.h>
unsigned long startTime;
#include <SD.h>
#include "Scenario.h"
#include "servo_control.h"

#define SERVOS 3  //Liczba serw fizycznie możliwych do podłączenia

//bool servoEnable[SERVOS] = {1, 1, 1};
const String filePrefix = "Servo";  //Prefiks pliku scenariusza



int servoIndex[SERVOS];  //Numer serwomechanizmu z nazwy pliku
int fileSize[SERVOS];    //Liczba linijek scenariusza
int numOfFiles = 0;      //Liczba plików scenariusza na karcie SD

bool allFilesEnded = 1; //Flaga czy wszystkie pliki się skończyły; 1 - skończył się, 0 - nie



void setup() {
  Serial.begin(115200);

  

  pinMode(0, INPUT_PULLUP);
  while (digitalRead(0)) {}  // CZEKA NA PRZYCISK ID0
  delay(2000);               //roboczo
  Serial.println(sizeof(int));  // size in bytes
  while (!SD.begin()) {
    Serial.println("Error: Card error");
    while (digitalRead(0)) {}
    delay(100);
  }

  File root = SD.open("/");

  while (!root) {
    Serial.println("Error: Failed to open main directory");
    while (digitalRead(0)) {}
    delay(100);
  }



  while (true) {  //Zbieranie danych o plikach scenariusza
    File file_temp = root.openNextFile();
    if (file_temp) {
      String fileName = file_temp.name();
      if (fileName.startsWith(filePrefix)) {
        fileSize[numOfFiles] = countLines(file_temp);
        servoIndex[numOfFiles] = 0;
        if (isDigit(fileName[filePrefix.length() + 1])) {
          servoIndex[numOfFiles] = fileName.substring(filePrefix.length(), filePrefix.length() + 2).toInt();
        } else {
          servoIndex[numOfFiles] = fileName[filePrefix.length()] - '0';
        }

        serialPrintFileData(fileName, fileSize[numOfFiles], servoIndex[numOfFiles]);

        numOfFiles++;
      }

    } else {
      break;
    }
  }
  root.close();

  Scenario** scenarios = new Scenario*[numOfFiles];
  for (int scenarioIndex = 0; scenarioIndex < numOfFiles; scenarioIndex++) {
     scenarios[scenarioIndex] = new Scenario(fileSize[scenarioIndex]);
     scenarios[scenarioIndex]->fileSize = &fileSize[scenarioIndex];
  }

  for (int scenarioIndex = 0; scenarioIndex < numOfFiles; scenarioIndex++) {  //Przpisywanie .csv do pamięci
    Serial.println("\n" + filePrefix + servoIndex[scenarioIndex] + ".csv");

    File file = SD.open("/" + filePrefix + servoIndex[scenarioIndex] + ".csv", FILE_READ);

    int lineIndex = 0;
    for (int lineIndex = 0; lineIndex < fileSize[scenarioIndex] && file.available(); lineIndex++) {
      scenarios[scenarioIndex]->timeStamps[lineIndex] = file.readStringUntil(';').toInt();
      if (lineIndex != 0 && scenarios[scenarioIndex]->timeStamps[lineIndex] == 0) {
        Serial.println("Error: Incorrect timestamp");
        break;
      }
      scenarios[scenarioIndex]->positions[lineIndex] = file.readStringUntil('\n').toInt();

      serialPrintLine(scenarios[scenarioIndex]->timeStamps[lineIndex], scenarios[scenarioIndex]->positions[lineIndex], scenarioIndex, lineIndex); //Robocze, wyświetla linijkę pliku
    }
  }
  setStartPosition(scenarios, numOfFiles);

  Serial.println("\nSTART SCENARIO?\n");
  while (digitalRead(0)) {}  // CZEKA NA PRZYCISK ID0
  delay(500);                //roboczo

  startScenarios(scenarios, numOfFiles, startTime, allFilesEnded);
    
  while(handleScenarios(scenarios, numOfFiles, startTime, allFilesEnded));

  Serial.println("\nEND OF ALL FILES.");
  
  delete[] scenarios;
  Serial.println("Deleted dynamic arrays");  //roboczo
}

void loop() {
}

