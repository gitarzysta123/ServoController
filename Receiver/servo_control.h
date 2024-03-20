
int countLines(File _file);
void serialPrintFileData(String _file_name, int _file_size, int _servo_index);
void serialPrintLine(int _time, int _position, int _array_index, int _line_index);
void serialPrintPositionLine(unsigned long _time1, int _array_index, int _current_line, int _position);
void calcStepDir(Scenario* scenarios[], int _array_index, unsigned long _time_start);