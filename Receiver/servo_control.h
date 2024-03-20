void serialPrintFileData(String _file_name, int _file_size, int _servo_index);
void serialPrintLine(int _time, int _position, int _array_index, int _line_index);
void serialPrintPositionLine(unsigned long _time1, int _array_index, int _current_line, int _position);
void setStartPosition(Scenario* scenarios[], int _num_of_files);
void startScenarios(Scenario* scenarios[], int _num_of_files, unsigned long& _start_time, bool& _all_files_ended);
bool handleScenarios(Scenario* scenarios[], int _num_of_files, unsigned long& _start_time, bool& _all_files_ended);

void calcStepDir(Scenario* scenarios[], int _scenario_index, int _start_time);
int countLines(File _file);