float fsr1, fsr2, fsr3, fsr4;

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);//fsr1 입력
  pinMode(A1, INPUT);//fsr2 입력
  pinMode(A2, INPUT);//fsr3 입력
  pinMode(A3, INPUT);//fsr4 입력

  pinMode(13, OUTPUT);
  
  fsr1 = analogRead(A0);
  fsr2 = analogRead(A1);
  fsr3 = analogRead(A2);
  fsr4 = analogRead(A3);
}

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

void loop() {
  bool fsr1set = seatState(A0, &fsr1);
  bool fsr2set = seatState(A1, &fsr2);
  bool fsr3set = seatState(A2, &fsr3);
  bool fsr4set = seatState(A3, &fsr4);
  //앉아있지 않다
  if(fsr1 < 5 && fsr2 < 5 && fsr3 < 5 && fsr4 < 5) {
    Serial.println("There's no person.");
    digitalWrite(13,LOW);
  }
  //앞쪽으로 치우쳐 앉는다
  else if(fsr1 < 50 || fsr2 < 50) {
    Serial.println("Put hip on the end of seat.");
    Serial.print(fsr1);Serial.print(" ");
    Serial.print(fsr2);Serial.print(" ");
    Serial.print(fsr3);Serial.print(" ");
    Serial.print(fsr4);Serial.println("");
    digitalWrite(13,HIGH);
  }
  //다리를 꼰다
  else if(fsr3 < 50 || fsr4 < 50) {
    Serial.println("Don't cross legs.");
    Serial.print(fsr1);Serial.print(" ");
    Serial.print(fsr2);Serial.print(" ");
    Serial.print(fsr3);Serial.print(" "); 
    Serial.print(fsr4);Serial.println("");
    digitalWrite(13,HIGH);
  }
  //엎드려 잔다
  else if(fsr3 > fsr1 || fsr4 > fsr2) {
    Serial.println("Don't sleep..");
    Serial.print(fsr1);Serial.print(" ");
    Serial.print(fsr2);Serial.print(" ");
    Serial.print(fsr3);Serial.print(" ");
    Serial.print(fsr4);Serial.println("");
    digitalWrite(13,HIGH);
  }
  //좌우 균형
  else if(fsr1*0.8 > fsr2 || fsr3*0.8 > fsr4) {
    Serial.println("Left sided.");
    Serial.print(fsr1);Serial.print(" ");
    Serial.print(fsr2);Serial.print(" ");
    Serial.print(fsr3);Serial.print(" ");
    Serial.print(fsr4);Serial.println("");
    digitalWrite(13,HIGH);
  }
  else if(fsr2*0.8 > fsr1 || fsr4*0.8 > fsr3) {
    Serial.println("Right sided.");
    Serial.print(fsr1);Serial.print(" ");
    Serial.print(fsr2);Serial.print(" ");
    Serial.print(fsr3);Serial.print(" ");
    Serial.print(fsr4);Serial.println("");
    digitalWrite(13,HIGH);
  }
  else {
    Serial.println("Seat proper state.");
    digitalWrite(13,LOW);
  }
  delay(100);
}
