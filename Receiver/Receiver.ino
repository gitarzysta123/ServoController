#include <SPI.h>
#include <SD.h>

#include "servo_control.h"

#define SERVOS 3  //Liczba serw fizycznie możliwych do podłączenia

//bool servoEnable[SERVOS] = {1, 1, 1};
const String filePrefix = "Servo";  //Prefiks pliku scenariusza


int servoIndex[SERVOS];  //Numer serwomechanizmu
int fileSize[SERVOS];    //Liczba linijek scenariusza
int numOfFiles = 0;      //Liczba plików scenariusza na karcie SD


void setup() {
  Serial.begin(115200);

  pinMode(0, INPUT_PULLUP);
  while (digitalRead(0)) {}  // CZEKA NA PRZYCISK ID0
  delay(2000);               //roboczo

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

  int **timeStamps = new int *[numOfFiles];  //[plik][linijka] - zapis wszystkich czasów z pliku
  int **positions = new int *[numOfFiles];  //[plik][linijka] - zapis wszystkich pozycji serwo z pliku
  int *currentLine = new int[numOfFiles]; //iterator wskazujący linię pliku do której przemieszcza się serwo
  int *currentPosition = new int[numOfFiles]; //obecna pozycja serwa
  int *timeStep = new int[numOfFiles]; //krok czasowy - co tyle msec serwo przesuwa się o 1
  int *direction = new int[numOfFiles]; //kierunek ruchu serwo
  unsigned long *timeC = new unsigned long[numOfFiles]; //znacznik czasowy zawierający ostatni czas ruchu serwo
  bool *fileEnded = new bool[numOfFiles]; //flaga czy dany plik się skończył; 1 - skończył się; 0 - nie
  bool allFilesEnded = 0; // -//- czy wszystkie pliki -//-
  for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
    timeStamps[arrayIndex] = new int[fileSize[arrayIndex]];
    positions[arrayIndex] = new int[fileSize[arrayIndex]];
  }

  for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {  //Przpisywanie .csv do pamięci
    Serial.println("\n" + filePrefix + servoIndex[arrayIndex] + ".csv");

    File file = SD.open("/" + filePrefix + servoIndex[arrayIndex] + ".csv", FILE_READ);

    int lineIndex = 0;
    while (file.available()) {
      timeStamps[arrayIndex][lineIndex] = file.readStringUntil(';').toInt();
      if (lineIndex != 0 && timeStamps[arrayIndex][lineIndex] == 0) {
        break;
      }
      positions[arrayIndex][lineIndex] = file.readStringUntil('\n').toInt();

      serialPrintLine(timeStamps[arrayIndex][lineIndex], positions[arrayIndex][lineIndex], arrayIndex, lineIndex); //Robocze, wyświetla linijkę pliku
      lineIndex++;
    }

    if (lineIndex != fileSize[arrayIndex]) {
      Serial.println("Error: Array lenght not equal to file length");
    }
  }


  for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) { //Przygotowanie do startu scenariusza
    currentLine[arrayIndex] = 1;
    fileEnded[arrayIndex] = false;
    currentPosition[arrayIndex] = positions[arrayIndex][0];
    serialPrintPositionLine(millis(), arrayIndex, 0, currentPosition[arrayIndex]); //Robocze, ustawienie pozycji początkowej, do zastąpienia przez servo.write
  }

  Serial.println("\nSTART SCENARIO?\n");
  while (digitalRead(0)) {}  // CZEKA NA PRZYCISK ID0
  delay(500);                //roboczo

  unsigned long time1 = millis(); //czas startu scenariusza

   for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
    timeC[arrayIndex] = millis();
    calcStepDir(arrayIndex, currentLine, positions, direction, timeStep, timeStamps, timeC, time1);
  }
  
  while (!allFilesEnded) {  //czytanie pozycji w czasie
    for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
      if ((millis() >= (timeC[arrayIndex] + timeStep[arrayIndex])) && (fileEnded[arrayIndex] == false)) {
        currentPosition[arrayIndex] += direction[arrayIndex];
        timeC[arrayIndex] = millis();

        serialPrintPositionLine(millis() - time1, arrayIndex, currentLine[arrayIndex], currentPosition[arrayIndex]); //Robocze, do zastąpienia przez servo.write

        if (currentPosition[arrayIndex] == positions[arrayIndex][currentLine[arrayIndex]]) {
          currentLine[arrayIndex]++;

          if (currentLine[arrayIndex] == fileSize[arrayIndex]) {
            fileEnded[arrayIndex] = true;

            allFilesEnded = 1;
            for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
              allFilesEnded = allFilesEnded * fileEnded[arrayIndex];
            }
          } else {
            calcStepDir(arrayIndex, currentLine, positions, direction, timeStep, timeStamps, timeC, time1); //Obliczenie kroku czasu, kierunku i czasu startu ruchu
          }
        }
      }
    }
  }


  Serial.println("\nEND OF ALL FILES.");
  
  
  for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) { //usuwanie dynamicznych
    delete[] timeStamps[arrayIndex];
    delete[] positions[arrayIndex];
  }
  delete[] timeStamps;  
  delete[] positions;
  delete[] currentLine;
  delete[] currentPosition;
  delete[] timeStep;
  delete[] direction;
  delete[] timeC;
  delete[] fileEnded;
  Serial.println("Deleted dynamic arrays");  //roboczo
}

void loop() {
}

