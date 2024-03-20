struct Scenario{
  int currentLine; //iterator wskazujący linię pliku do której przemieszcza się serwo
  int currentPosition; //obecna pozycja serwa
  int timeStep; //krok czasowy - co tyle msec serwo przesuwa się o 1
  int direction; //kierunek ruchu serwo
  unsigned long lastTime; //znacznik czasowy zawierający ostatni czas ruchu serwo
  bool fileEnded = 1; //flaga czy dany plik się skończył; 1 - skończył się; 0 - nie

  int* fileSize;

  int* timeStamps;
  int* positions;
  Scenario();
  Scenario(int _file_size){
    timeStamps = new int[_file_size];
    positions = new int[_file_size];
  }
 // private: Scenario();
 ~Scenario() {
        delete[] timeStamps;
        delete[] positions;
  }
};