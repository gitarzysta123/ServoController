#include <SPI.h>
#include <SD.h>

#define SERVOS 3  //Liczba serw fizycznie możliwych do podłączenia

//bool servoEnable[SERVOS] = {1, 1, 1};
const String filePrefix = "Servo";  //Prefiks pliku scenariusza


int servoIndex[SERVOS];         //Numer serwomechanizmu
unsigned int fileSize[SERVOS];  //Liczba linijek scenariusza
int numOfFiles = 0;             //Liczba plików scenariusza na karcie SD

void setup() {
  Serial.begin(115200);

  pinMode(0, INPUT_PULLUP);
  while (digitalRead(0)) {}
  delay(2000);

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

        Serial.println("Found scenario file: " + fileName);
        Serial.print("Size: ");
        Serial.print(fileSize[numOfFiles]);
        Serial.print("  Servo index: ");
        Serial.println(servoIndex[numOfFiles]);

        numOfFiles++;
      }

    } else {
      break;
    }
  }
  root.close();

  unsigned int **timeStamps = new unsigned int *[numOfFiles];  //Alokacja pamięci
  int **positions = new int *[numOfFiles];
  for (int i = 0; i < numOfFiles; i++) {
    timeStamps[i] = new unsigned int[fileSize[i]];
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
      serialPrintLine(timeStamps, positions, arrayIndex, lineIndex);
      lineIndex++;
    }

    if (lineIndex != fileSize[arrayIndex]) {
      Serial.println("Error: Array lenght not equal to file length");
    }
  }

  delay(1000);
  Serial.println("\nSTART SCENARIO?\n");
  while (digitalRead(0)) {}

  Serial.print("time");  // Convert millis() to a String
        Serial.print("\t");              // Tab character
        Serial.print("file");
        Serial.print("\t");  // Tab character
        Serial.print("line");
        Serial.print("\t");  // Tab character
        Serial.println("pos");

  int *currentLine = new int[numOfFiles];
  bool *stop = new bool[numOfFiles];
  bool STOP = 0;
  for (int i = 0; i < numOfFiles; i++) {
    currentLine[i] = 0;
    stop[i] = false;
  }

  unsigned long time1 = millis();

  while (!STOP) {
    for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
      if ((millis() - time1) >= timeStamps[arrayIndex][currentLine[arrayIndex]] && stop[arrayIndex] == 0) {

        Serial.print(String(millis() - time1));  // Convert millis() to a String
        Serial.print("\t");              // Tab character
        Serial.print(arrayIndex);
        Serial.print("\t");  // Tab character
        Serial.print(currentLine[arrayIndex]);
        Serial.print("\t");  // Tab character
        Serial.println(positions[arrayIndex][currentLine[arrayIndex]]);
        
        currentLine[arrayIndex]++;
        if (currentLine[arrayIndex] == fileSize[arrayIndex]) {
          stop[arrayIndex] = true;
        }
      }
    }

    STOP = 1;
    for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
      STOP = STOP * stop[arrayIndex];
    }
  }
}

void loop() {
}

unsigned int countLines(File file) {
  unsigned int lineCount = 0;
  bool wasEmpty = 0;
  char c;
  while (file.available()) {
    c = file.read();
    if (c != '\r') {
      if (c == '\n') {
        if (!wasEmpty) {
          lineCount++;
          wasEmpty = true;
        } else {
          return lineCount;
        }
      } else {
        wasEmpty = false;
      }
    }
  }
  if (!wasEmpty) {
    lineCount++;
  }
  return lineCount;
}

void serialPrintLine(unsigned int **array1, int **array2, int array_index, unsigned int line_index) {
  Serial.print(array_index);
  Serial.print(" , ");
  Serial.print(line_index);
  Serial.print(" : ");
  Serial.print(array1[array_index][line_index]);
  Serial.print(";");
  Serial.println(array2[array_index][line_index]);
}