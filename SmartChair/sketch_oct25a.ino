extern float fsr1, fsr2, fsr2, fsr4;

bool seatState(uint8_t pin_in, float* fsr) {
  float result = analogRead(pin_in);
  
  if((*fsr)*1.1 >= result && (*fsr)*0.9 <= result) {
    *fsr = (result + (*fsr)) / 2;
    return true;
  }
  else {
    *fsr = result;
    return false;
  }
}

int getPose() {
  bool fsr1set = seatState(A0, &fsr1);
  bool fsr2set = seatState(A1, &fsr2);
  bool fsr3set = seatState(A2, &fsr3);
  bool fsr4set = seatState(A3, &fsr4);

  //앉아있지 않다
  if(fsr1 < 5 && fsr2 < 5 && fsr3 < 5 && fsr4 < 5) {
    //Serial.println("There's no person.");
    return NO_SEAT;
  }
  //앞쪽으로 치우쳐 앉는다
  else if(fsr1 < 50 || fsr2 < 50) {
    //Serial.println("Put hip on the end of seat.");
    return FRONT_SIDED;
  }
  //다리를 꼰다
  else if(fsr3 < 50 || fsr4 < 50) {
    //Serial.println("Don't cross legs.");
    return CROSS_LEG;
  }
  //엎드려 잔다
  else if(fsr3 > fsr1 || fsr4 > fsr2) {
    //Serial.println("Don't sleep..");
    return SLEEP;
  }
  //좌우 균형
  else if(fsr1*0.8 > fsr2 || fsr3*0.8 > fsr4) {
    //Serial.println("Left sided.");
    return LEFT_SIDED;
  }
  else if(fsr2*0.8 > fsr1 || fsr4*0.8 > fsr3) {
    //Serial.println("Right sided.");
    return RIGHT_SIDED;
  }
  else {
    //Serial.println("Seat proper state.");
    return GOOD;
  }
}
