#include <SPI.h>
#include <SD.h>
#include "Scenario.h"
#include "servo_control.h"


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

void calcStepDir(Scenario* scenarios[], int _scenario_index, int _start_time) {
  int _last_line = (scenarios[_scenario_index]->currentLine) - 1;
  int posDiff = scenarios[_scenario_index]->positions[scenarios[_scenario_index]->currentLine] - scenarios[_scenario_index]->positions[_last_line];
  if (posDiff > 0) {
    scenarios[_scenario_index]->direction = 1;
  } else if (posDiff < 0) {
    scenarios[_scenario_index]->direction = -1;
  } else {
    scenarios[_scenario_index]->direction = 0;
  }

  posDiff = abs(posDiff);

  if (posDiff != 0) {
    scenarios[_scenario_index]->timeStep = (scenarios[_scenario_index]->timeStamps[scenarios[_scenario_index]->currentLine] - scenarios[_scenario_index]->timeStamps[_last_line]) / posDiff;
  } else {
    scenarios[_scenario_index]->timeStep = scenarios[_scenario_index]->timeStamps[scenarios[_scenario_index]->currentLine] - scenarios[_scenario_index]->timeStamps[_last_line];
  }

  Serial.print("Timestep ");
  Serial.print(_scenario_index);
  Serial.print(" : ");
  Serial.println(scenarios[_scenario_index]->timeStep);

  scenarios[_scenario_index]->lastTime = _start_time + (unsigned long)(scenarios[_scenario_index]->timeStamps[_last_line]);
}

void serialPrintFileData(String _file_name, int _file_size, int _servo_index) {
  Serial.println("\nFound scenario file: " + _file_name);
  Serial.print("Size: ");
  Serial.print(_file_size);
  Serial.print("  Servo index: ");
  Serial.println(_servo_index);
}

void serialPrintLine(int _time, int _position, int _scenario_index, int _line_index) {
  Serial.print(_scenario_index);
  Serial.print(" , ");
  Serial.print(_line_index);
  Serial.print(" : ");
  Serial.print(_time);
  Serial.print(";");
  Serial.println(_position);
}

void serialPrintPositionLine(unsigned long _time1, int _scenario_index, int _current_line, int _position) {
  Serial.print(String(_time1));
  Serial.print("\t");
  Serial.print(_scenario_index);
  Serial.print("\t");
  Serial.print(_current_line);
  Serial.print("\t");
  Serial.println(_position);
}

void getFilesData(String _file_prefix, int& _num_of_files, int _file_size[], int _servo_index[]) {
  _num_of_files = 0;

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
      if (fileName.startsWith(_file_prefix)) {
        _file_size[_num_of_files] = countLines(file_temp);
        _servo_index[_num_of_files] = 0;
        if (isDigit(fileName[_file_prefix.length() + 1])) {
          _servo_index[_num_of_files] = fileName.substring(_file_prefix.length(), _file_prefix.length() + 2).toInt();
        } else {
          _servo_index[_num_of_files] = fileName[_file_prefix.length()] - '0';
        }

        serialPrintFileData(fileName, _file_size[_num_of_files], _servo_index[_num_of_files]);

        _num_of_files++;
      }

    } else {
      break;
    }
  }
  root.close();
}

bool copyFiles(Scenario* scenarios[], String _file_prefix, int _num_of_files, int _file_size[], int _servo_index[]) {
  for (int scenarioIndex = 0; scenarioIndex < _num_of_files; scenarioIndex++) {  //Przpisywanie .csv do pamięci
    Serial.println("\n" + _file_prefix + _servo_index[scenarioIndex] + ".csv");

    File file = SD.open("/" + _file_prefix + _servo_index[scenarioIndex] + ".csv", FILE_READ);

    int lineIndex = 0;
    for (int lineIndex = 0; lineIndex < _file_size[scenarioIndex] && file.available(); lineIndex++) {
      scenarios[scenarioIndex]->timeStamps[lineIndex] = file.readStringUntil(';').toInt();
      if (lineIndex != 0 && scenarios[scenarioIndex]->timeStamps[lineIndex] == 0) {
        Serial.println("Error: Incorrect timestamp");
        return false;
      }
      scenarios[scenarioIndex]->positions[lineIndex] = file.readStringUntil('\n').toInt();

      serialPrintLine(scenarios[scenarioIndex]->timeStamps[lineIndex], scenarios[scenarioIndex]->positions[lineIndex], scenarioIndex, lineIndex);  //Robocze, wyświetla linijkę pliku
    }
  }
  return true;
}

void setStartPosition(Scenario* scenarios[], int _num_of_files) {
  Serial.println("time\tfile\tline\tpos");
  for (int scenarioIndex = 0; scenarioIndex < _num_of_files; scenarioIndex++) {  //Przygotowanie do startu scenariusza
    scenarios[scenarioIndex]->currentLine = 1;
    scenarios[scenarioIndex]->currentPosition = scenarios[scenarioIndex]->positions[0];
    serialPrintPositionLine(millis(), scenarioIndex, 0, scenarios[scenarioIndex]->currentPosition);  //Robocze, ustawienie pozycji początkowej, do zastąpienia przez servo.write
  }
}

void startScenarios(Scenario* scenarios[], int _num_of_files, unsigned long& _start_time, bool& _all_files_ended) {
  _start_time = millis();  //czas startu scenariusza
  _all_files_ended = 0;

  for (int scenarioIndex = 0; scenarioIndex < _num_of_files; scenarioIndex++) {
    scenarios[scenarioIndex]->lastTime = millis();
    calcStepDir(scenarios, scenarioIndex, _start_time);  //Obliczenie kroku czasu, kierunku i czasu startu ruchu
    scenarios[scenarioIndex]->fileEnded = false;
  }
  Serial.println("time\tfile\tline\tpos");
}

bool handleScenarios(Scenario* scenarios[], int _num_of_files, unsigned long& _start_time, bool& _all_files_ended) {
  if (!_all_files_ended) {  //czytanie(w przyszłości serwo.write) pozycji w czasie
    for (int scenarioIndex = 0; scenarioIndex < _num_of_files; scenarioIndex++) {
      if ((millis() >= (scenarios[scenarioIndex]->lastTime + scenarios[scenarioIndex]->timeStep)) && (scenarios[scenarioIndex]->fileEnded == false)) {
        scenarios[scenarioIndex]->currentPosition += scenarios[scenarioIndex]->direction;
        scenarios[scenarioIndex]->lastTime = millis();

        serialPrintPositionLine(millis() - _start_time, scenarioIndex, scenarios[scenarioIndex]->currentLine, scenarios[scenarioIndex]->currentPosition);  //Robocze, do zastąpienia przez servo.write

        if (scenarios[scenarioIndex]->currentPosition == scenarios[scenarioIndex]->positions[scenarios[scenarioIndex]->currentLine]) {
          scenarios[scenarioIndex]->currentLine++;

          if (scenarios[scenarioIndex]->currentLine == scenarios[scenarioIndex]->fileSize) {
            scenarios[scenarioIndex]->fileEnded = true;

            _all_files_ended = 1;
            for (int scenarioIndex = 0; scenarioIndex < _num_of_files; scenarioIndex++) {
              _all_files_ended = _all_files_ended && scenarios[scenarioIndex]->fileEnded;
            }
          } else {
            calcStepDir(scenarios, scenarioIndex, _start_time);  //Obliczenie kroku czasu, kierunku i czasu startu ruchu
          }
        }
      }
    }
  }
  return !_all_files_ended;
}
