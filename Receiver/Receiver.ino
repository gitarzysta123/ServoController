#include <SPI.h>
#include <SD.h>

#define SERVOS 3  //Liczba serw fizycznie możliwych do podłączenia

//bool servoEnable[SERVOS] = {1, 1, 1};
const String filePrefix = "Servo";

unsigned long time1 = 0;
int lineCount[SERVOS];
int servoIndex[SERVOS];
unsigned int fileSize[SERVOS];
int numOfFiles = 0;
unsigned int longestFileSize = 0;

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



  while (true) {
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

  longestFileSize = getMaxLength(fileSize, numOfFiles);
  Serial.print("\nMAX LENGTH: ");
  Serial.println(longestFileSize);

  unsigned int timeStamps[numOfFiles][longestFileSize];
  int positions[numOfFiles][longestFileSize];

  for (int arrayIndex = 0; arrayIndex < numOfFiles; arrayIndex++) {
    Serial.println("\n" + filePrefix + servoIndex[arrayIndex] + ".csv");
    File file = SD.open("/" + filePrefix + servoIndex[arrayIndex] + ".csv", FILE_READ);
    int lineIndex = 0;
    while (file.available()) {
      timeStamps[arrayIndex][lineIndex] = file.readStringUntil(';').toInt();
      if (lineIndex != 0 && timeStamps[arrayIndex][lineIndex] == 0){
        break;
      }
      positions[arrayIndex][lineIndex] = file.readStringUntil('\n').toInt();
      Serial.print(arrayIndex);
      Serial.print(" , ");
      Serial.print(lineIndex);
      Serial.print(" : ");
      Serial.print(timeStamps[arrayIndex][lineIndex]);
      Serial.print(";");
      Serial.println(positions[arrayIndex][lineIndex]);
      lineIndex++;
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

unsigned int getMaxLength(unsigned int file[], int num) {
  unsigned int maxLength = file[0];
  for (int i = 1; i < num; i++) {
    if (file[i] > maxLength) {
      maxLength = file[i];
    }
  }
  return maxLength;
}