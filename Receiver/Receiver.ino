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
bool allFilesEnded = 0; //Flaga czy wszystkie pliki się skończyły; 1 - skończył się, 0 - nie



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
  }
  // int **timeStamps = new int *[numOfFiles];  //[plik][linijka] - zapis wszystkich czasów z pliku
  // int **positions = new int *[numOfFiles];  //[plik][linijka] - zapis wszystkich pozycji serwo z plikureadStringUntil
  // int *currentLine = new int[numOfFiles]; //iterator wskazujący linię pliku do której przemieszcza się serwo/
  // int *currentPosition = new int[numOfFiles]; //obecna pozycja serwa/
  // int *timeStep = new int[numOfFiles]; //krok czasowy - co tyle msec serwo przesuwa się o 1/
  // int *direction = new int[numOfFiles]; //kierunek ruchu serwo/
  // unsigned long *timeC = new unsigned long[numOfFiles]; //znacznik czasowy zawierający ostatni czas ruchu serwo/
  // bool *fileEnded = new bool[numOfFiles]; //flaga czy dany plik się skończył; 1 - skończył się; 0 - nie/
  
  // for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
  //   timeStamps[arrayIndex] = new int[fileSize[arrayIndex]];
  //   positions[arrayIndex] = new int[fileSize[arrayIndex]];
  // }

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

  Serial.println("time\tfile\tline\tpos");

  for (int scenarioIndex = 0; scenarioIndex < numOfFiles; scenarioIndex++) { //Przygotowanie do startu scenariusza
    scenarios[scenarioIndex]->currentLine = 1;
    scenarios[scenarioIndex]->fileEnded = false;
    scenarios[scenarioIndex]->currentPosition = scenarios[scenarioIndex]->positions[0];
    serialPrintPositionLine(millis(), scenarioIndex, 0, scenarios[scenarioIndex]->currentPosition); //Robocze, ustawienie pozycji początkowej, do zastąpienia przez servo.write
  }

  Serial.println("\nSTART SCENARIO?\n");
  while (digitalRead(0)) {}  // CZEKA NA PRZYCISK ID0
  delay(500);                //roboczo

  unsigned long time1 = millis(); //czas startu scenariusza

   for (int scenarioIndex = 0; scenarioIndex < numOfFiles; scenarioIndex++) {
    scenarios[scenarioIndex]->lastTime= millis();
    calcStepDir(scenarios, scenarioIndex, time1);
  }
  
  while (!allFilesEnded) {  //czytanie(w przyszłości serwo.write) pozycji w czasie
    for (int scenarioIndex = 0; scenarioIndex < numOfFiles; scenarioIndex++) {
      if ((millis() >= (scenarios[scenarioIndex]->lastTime + scenarios[scenarioIndex]->timeStep)) && (scenarios[scenarioIndex]->fileEnded == false)) {
        scenarios[scenarioIndex]->currentPosition += scenarios[scenarioIndex]->direction;
        scenarios[scenarioIndex]->lastTime = millis();

        serialPrintPositionLine(millis() - time1, scenarioIndex, scenarios[scenarioIndex]->currentLine, scenarios[scenarioIndex]->currentPosition); //Robocze, do zastąpienia przez servo.write

        if (scenarios[scenarioIndex]->currentPosition == scenarios[scenarioIndex]->positions[scenarios[scenarioIndex]->currentLine]) {
          scenarios[scenarioIndex]->currentLine++;

          if (scenarios[scenarioIndex]->currentLine == fileSize[scenarioIndex]) {
            scenarios[scenarioIndex]->fileEnded = true;

            allFilesEnded = 1;
            for (int scenarioIndex = 0; scenarioIndex < numOfFiles; scenarioIndex++) {
              allFilesEnded = allFilesEnded * scenarios[scenarioIndex]->fileEnded;
            }
          } else {
            calcStepDir(scenarios, scenarioIndex, time1); //Obliczenie kroku czasu, kierunku i czasu startu ruchu
          }
        }
      }
    }
  }


  Serial.println("\nEND OF ALL FILES.");
  
  
  delete[] scenarios;
  Serial.println("Deleted dynamic arrays");  //roboczo
}

void loop() {
}

