#include <SD.h>
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
  Serial.print(String(_time1));
  Serial.print("\t");
  Serial.print(_array_index);
  Serial.print("\t");
  Serial.print(_current_line);
  Serial.print("\t");
  Serial.println(_position);
}

void calcStepDir(int _array_index, int _current_line[], int* _positions[], int _direction[], int _time_step[], int* _time_stamps[], unsigned long _time_mark[], unsigned long _time_start) {
  int _last_line = _current_line[_array_index] - 1;
  int posDiff = _positions[_array_index][_current_line[_array_index]] - _positions[_array_index][_last_line];
  if (posDiff > 0) {
    _direction[_array_index] = 1;
  } else if (posDiff < 0) {
    _direction[_array_index] = -1;
  } else {
    _direction[_array_index] = 0;
  }

  posDiff = abs(posDiff);

  if (posDiff != 0) {
    _time_step[_array_index] = (_time_stamps[_array_index][_current_line[_array_index]] - _time_stamps[_array_index][_last_line]) / posDiff;
  } else {
    _time_step[_array_index] = _time_stamps[_array_index][_current_line[_array_index]] - _time_stamps[_array_index][_last_line];
  }

  Serial.print("Timestep ");
  Serial.print(_array_index);
  Serial.print(" : ");
  Serial.println(_time_step[_array_index]);

  _time_mark[_array_index] = _time_start + (unsigned long)(_time_stamps[_array_index][_last_line]);
}