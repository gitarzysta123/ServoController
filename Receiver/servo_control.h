
int countLines(File _file);
void serialPrintFileData(String _file_name, int _file_size, int _servo_index);
void serialPrintLine(int _time, int _position, int _array_index, int _line_index);
void serialPrintPositionLine(unsigned long _time1, int _array_index, int _current_line, int _position);
void calcStepDir(int _array_index, int _current_line[], int* _positions[], int _direction[], int _time_step[], int* _time_stamps[], unsigned long _time_mark[], unsigned long _time_start);