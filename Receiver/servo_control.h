void getFilesData(String _file_prefix, int& _num_of_files, int _file_size[], int _servo_index[]);                    //Zbieranie danych o plikach scenariusza
bool copyFiles(Scenario* scenarios[], String _file_prefix, int _num_of_files);                                       //Przpisywanie .csv do pamięci
void setStartPosition(Scenario* scenarios[], int _num_of_files);                                                     //Przygotowanie do startu scenariusza, ustawienie pozycji startowych
void startScenarios(Scenario* scenarios[], int _num_of_files, unsigned long& _start_time, bool& _all_files_ended);   //Start scenartiuszy
bool handleScenarios(Scenario* scenarios[], int _num_of_files, unsigned long& _start_time, bool& _all_files_ended);  //Update wykonywania scenariuszy

void calcStepDir(Scenario* scenarios[], int _scenario_index, int _start_time);
int countLines(File _file);

void serialPrintFileData(String _file_name, int _file_size, int _servo_index);
void serialPrintLine(int _time, int _position, int _array_index, int _line_index);
void serialPrintPositionLine(unsigned long _time1, int _array_index, int _current_line, int _position);