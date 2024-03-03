#include <SPI.h>
#include <SD.h>

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

  int **timeStamps = new int *[numOfFiles];  //Alokacja pamięci
  int **positions = new int *[numOfFiles];
  for (int i = 0; i < numOfFiles; i++) {
    timeStamps[i] = new int[fileSize[i]];
    positions[i] = new int[fileSize[i]];
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

      serialPrintLine(timeStamps[arrayIndex][lineIndex], positions[arrayIndex][lineIndex], arrayIndex, lineIndex);
      lineIndex++;
    }

    if (lineIndex != fileSize[arrayIndex]) {
      Serial.println("Error: Array lenght not equal to file length");
    }
  }

  Serial.println("\nSTART SCENARIO?\n");
  while (digitalRead(0)) {}  // CZEKA NA PRZYCISK ID0
  delay(500);                //roboczo

  Serial.println("time\tfile\tline\tpos");

  int *currentLine = new int[numOfFiles];  //flagi, iteratory
  bool *fileEnded = new bool[numOfFiles];
  bool allFilesEnded = 0;
  for (int i = 0; i < numOfFiles; i++) {
    currentLine[i] = 0;
    fileEnded[i] = false;
  }

  unsigned long time1 = millis();

  while (!allFilesEnded) {  //czytanie pozycji w czasie
    for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
      if ((millis() - time1) >= timeStamps[arrayIndex][currentLine[arrayIndex]] && fileEnded[arrayIndex] == 0) {

        serialPrintPositionLine(time1, arrayIndex, currentLine[arrayIndex], positions[arrayIndex][currentLine[arrayIndex]]);

        currentLine[arrayIndex]++;
        if (currentLine[arrayIndex] == fileSize[arrayIndex]) {
          fileEnded[arrayIndex] = true;

          Serial.print("\nEND OF FILE ");
          Serial.println(arrayIndex);
          Serial.print("\n");
        }
      }
    }

    allFilesEnded = 1;
    for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
      allFilesEnded = allFilesEnded * fileEnded[arrayIndex];
    }
  }


  Serial.println("\nEND OF ALL FILES.");

  delete[] timeStamps;  //usuwanie dynamicznych
  delete[] positions;
  for (int i = 0; i < numOfFiles; i++) {
    delete[] timeStamps[i];
    delete[] positions[i];
  }
  delete[] currentLine;
  delete[] fileEnded;
  Serial.println("Deleted dynamic arrays");  //roboczo
}

void loop() {
}

int countLines(File _file) {  //liczenie linijek w pliku
  int _lineCount = 0;
  bool _wasEmpty = 0;
  char _c;
  while (_file.available()) {
    _c = _file.read();
    if (_c != '\r') {
      if (_c == '\n') {
        if (!_wasEmpty) {
          _lineCount++;
          _wasEmpty = true;
        } else {
          return _lineCount;
        }
      } else {
        _wasEmpty = false;
      }
    }
  }
  if (!_wasEmpty) {
    _lineCount++;
  }
  return _lineCount;
}

void serialPrintFileData(String _file_name, int _file_size, int _servo_index) {
  Serial.println("\nFound scenario file: " + _file_name);
  Serial.print("Size: ");
  Serial.print(_file_size);
  Serial.print("  Servo index: ");
  Serial.println(_servo_index);
}

void serialPrintLine(int _time, int _position, int _array_index, int _line_index) {
  Serial.print(_array_index);
  Serial.print(" , ");
  Serial.print(_line_index);
  Serial.print(" : ");
  Serial.print(_time);
  Serial.print(";");
  Serial.println(_position);
}

void serialPrintPositionLine(unsigned long _time1, int _array_index, int _current_line, int _position) {
  Serial.print(String(millis() - _time1));
  Serial.print("\t");
  Serial.print(_array_index);
  Serial.print("\t");
  Serial.print(_current_line);
  Serial.print("\t");
  Serial.println(_position);
}