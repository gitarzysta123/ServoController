struct Scenario {
  int currentLine;         //iterator wskazujący linię pliku do której przemieszcza się serwo
  int currentPosition;     //obecna pozycja serwa
  int timeStep;            //krok czasowy - co tyle msec serwo przesuwa się o 1
  int direction;           //kierunek ruchu serwo
  unsigned long lastTime;  //znacznik czasowy zawierający ostatni czas ruchu serwo
  bool fileEnded = 1;      //flaga czy dany plik się skończył; 1 - skończył się; 0 - nie

  int fileSize;
  int servoIndex;
  bool servoEnable;

  int* timeStamps;
  int* positions;
  Scenario(int _file_size, int _servo_index, bool _servo_enable) {
    servoEnable = _servo_enable;
    servoIndex = _servo_index;
    if (_servo_enable) {
      fileSize = _file_size;

      timeStamps = new int[fileSize];
      positions = new int[fileSize];
    }
  }
  // private: Scenario();
  ~Scenario() {
    delete[] timeStamps;
    delete[] positions;
  }
};