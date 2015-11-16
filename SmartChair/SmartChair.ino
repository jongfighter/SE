
class Timer
{
  public:
    bool isOn = false;
    long endTime;
    long curTime;

    Timer(long endTime_Second)
    {
      curTime = 0;
      endTime = endTime_Second * 1000;
    }

    Timer(float endTime_float)
    {
      curTime = 0;
      endTime = (long)(endTime_float * 1000);
    }

    void setTime(long endTime_Second)
    {
      endTime = endTime_Second * 60*1000;
    }

    void showTime()
    {
      Serial.println(curTime);
    }

    bool isTimeEnd()
    {
      if (curTime >= endTime)
        return true;
      else
        return false;
    }

    void reset()
    {
      curTime = 0;
      isOn = false;
    }

    void on()
    {
      isOn = true;
      prevT = (long)millis();
    }

    void off()
    {
      isOn = false;
    }

    void timeCheck()
    {
      if (isOn)
      {
        curT = (long)millis();
        deltaT = curT - prevT;
        curTime += deltaT;
        prevT = (long)millis();
      }
    }

  private :

    long curT = 0;
    long prevT = 0;
    long deltaT = 0;
};

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#include "seatState.h"

const int speakerOut = 9;
const int vibeOut = 3;
char incomingByte;  // incoming data
bool isStarted = false;
bool isRestTime = false;
int DEBUG = 1;
int temp = 0;
bool updown = true;


bool isAlarm = false;
bool isSitting = false;


long prev_time = 0;
long current_time = 0;
long delta_time = 0;
long timer_started = 0;
long timeEnd = 5000;
String rcvMsg;
String rcvByte;

// Set overall tempo
long tempo = 10000;
// Set length of pause between notes
int pause = 1000;
// Loop variable to increase Rest length
int rest_count = 100; //<-BLETCHEROUS HACK; See NOTES

// Initialize core variables
int tone_ = 0;
int beat = 0;
long duration  = 0;


int melody[] = {  NOTE_C7, NOTE_D7, NOTE_E7, NOTE_F7, NOTE_G7, NOTE_A6, NOTE_B5, NOTE_C6 };
int beats[]  = { 64, 64, 64, 64, 64, 64, 64, 64};
int MAX_COUNT = sizeof(melody) / 2; // Melody length, for looping.


Timer* timer_sit;
Timer* timer_leave;
Timer* timer_posture;
Timer* timer_rest;
Timer* timer_alarm;

//chair_pose
float fsr1, fsr2, fsr3, fsr4;

//소리 켜고 끄기
bool soundIsOn = false;

int myPin=0000;



void setup() {

  timer_sit = new Timer((long)5);
  timer_leave = new Timer((long)5);
  timer_posture = new Timer((long)5);
  timer_rest = new Timer((long)5);
  timer_alarm = new Timer(0.0014f);

  if (DEBUG) {
    Serial.begin(9600); // initialization
  }

  pinMode(vibeOut, OUTPUT);//진동 입력
  pinMode(speakerOut, OUTPUT);
  Serial.println("Press 1 to LED ON or 0 to LED OFF...");

  //chair_pose
  pinMode(A0, INPUT);//fsr1 입력
  pinMode(A1, INPUT);//fsr2 입력
  pinMode(A2, INPUT);//fsr3 입력
  pinMode(A3, INPUT);//fsr4 입력

  fsr1 = analogRead(A0);
  fsr2 = analogRead(A1);
  fsr3 = analogRead(A2);
  fsr4 = analogRead(A3);
}

bool isOn(uint8_t pin_in)
{
  int data = digitalRead(pin_in);

  if (data) return true;
  else return false;
}


// PLAY TONE  ==============================================
// Pulse the speaker to play a tone for a particular duration
void playTone() {
  long elapsed_time = 0;
  if (tone_ > 0) { // if this isn't a Rest beat, while the tone has
    //  played less long than 'duration', pulse speaker HIGH and LOW
    while (elapsed_time < duration) {

      digitalWrite(speakerOut, HIGH);
      delayMicroseconds(tone_ / 2);

      // DOWN
      digitalWrite(speakerOut, LOW);
      delayMicroseconds(tone_ / 2);

      // Keep track of how long we pulsed
      elapsed_time += (tone_);
    }
  }
  else { // Rest beat; loop times delay
    for (int j = 0; j < rest_count; j++) { // See NOTE on rest_count
      delayMicroseconds(duration);
    }
  }
}

//isOn, playTone 함수 사용 가능
void loop() {

  timer_alarm->timeCheck();
  timer_sit->timeCheck();
  timer_leave->timeCheck();
  timer_posture->timeCheck();
  timer_rest->timeCheck();
  int num = digitalRead(vibeOut);
  current_time = (long) millis();
  delta_time = current_time - prev_time;

  if (isStarted) {
    isStarted = false;
    timer_sit->reset();
    timer_sit->on();
    timer_sit->showTime();
  }

  if (Serial.available() > 0) {  // if the data came
    rcvMsg = Serial.readString(); // read byte
    /*
        rcvByte[0]=incomingByte;
        rcvByte[1]='\0';
        strcat(rcvMsg,rcvByte);
        Serial.println(rcvMsg);
      */

    if(rcvMsg[0]=='P'&& rcvMsg[1] == 'I' && rcvMsg[2]=='N')
    {
      String rcvPin_str= rcvMsg.substring(3);
      int rcvPin=atoi(rcvPin_str.c_str());
      if(rcvPin==myPin)
      Serial.println("LOGIN");
      else
      Serial.println("DENY");
    }
    
    if (rcvMsg[0] == 'S' && rcvMsg[1] == 'E' && rcvMsg[2] == 'T')
    {
      String timeString = rcvMsg.substring(3);
      Serial.println(timeString);
      timer_sit->setTime((long)atoi(timeString.c_str()));
      Serial.println("ACK");
    }
    if (rcvMsg[0] == '0') {
      Serial.println("Received!");  // print message
    }
    if (rcvMsg[0] == 'D' && rcvMsg[1] == 'O') {
      isStarted = true;
    }

    if (rcvMsg[0] == '2') {
      timer_sit->showTime();
    }
  }
  
  /* 앉아있을 시간이 지나지 않았는데 일어나면 알람이 울린다
  if (isSitting == false && timer_sit->isOn) {
    isAlarm = true;
    timer_sit->reset();
  }
  */
  
  if (timer_sit->isTimeEnd()) {
    isAlarm = true;
    isRestTime = true;
    timer_sit->reset();
    Serial.println("LONG");
  }

  /* 쉬는 시간이 지나면 알람이 울려 자리에 앉도록 한다
  if (timer_rest->isTimeEnd()) {
    isAlarm = true;
    isRestTime = false;
    timer_rest->reset();
    Serial.println("SIT");
  }
  */
  if (isAlarm)
  if (isSitting && isRestTime == false)
  {
    isAlarm = false;
  }
  else if (isSitting == false && isRestTime)
  {
    isAlarm = false;
    /* 자리에서 일어나면 휴식시간을 재는 타이머가 작동한다.
    timer_rest->reset();
    timer_rest->on();
    timer_rest->showTime();
    */
  }

  if (isAlarm)
  {
    /* timer_alarm->on();
     if(timer_alarm->isTimeEnd())
     {
       updown=!updown;
       timer_alarm->reset();
       timer_alarm->on();
     }*/
    if (updown)
    {
      updown = !updown;
      digitalWrite(speakerOut, LOW);
    }
    else
    {
      updown = !updown;
      digitalWrite(speakerOut, HIGH);
    }
  }

  switch (getPose()) {
    case NO_SEAT:
      {
        isStarted = false;
        isSitting = false;
        soundIsOn = false;
        break;
      }
    case FRONT_SIDED: soundIsOn = true; break;
    case CROSS_LEG: soundIsOn = true; break;
    case SLEEP: soundIsOn = true; break;
    case LEFT_SIDED: soundIsOn = true; break;
    case RIGHT_SIDED: soundIsOn = true; break;
    case GOOD:
      { 
        if(isSitting == false) {
          isSitting = true;
          isStarted = true; 
        }
        soundIsOn = false;
        break;
      }
deafult: break;
  }

  prev_time = current_time;

  // Set up a counter to pull from melody[] and beats[]
  if (soundIsOn)
    digitalWrite(vibeOut, HIGH);
  else
    digitalWrite(vibeOut, LOW);
  /*
  for (int i = 0; i < MAX_COUNT; i++) {
    playTone();
    tone_ = melody[i];
    beat = beats[i];

    duration = beat * tempo; // Set up timing

  }
  */
}
