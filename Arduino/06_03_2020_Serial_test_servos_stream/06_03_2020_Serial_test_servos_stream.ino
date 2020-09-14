#include <Servo.h>


String prijate; //i.e. received
float sampleData;
String sampleDataStr;
boolean activateStream;
int servoPos[4] = {90,90,90,90};
int servoPins[] = {3,6,9,10};  //Edit if necessary
int lastSent=0;
Servo servo1, servo2, servo3, servo4;

void sendData() {
  float argument, accX, accY, accZ, gyrX, gyrY ,gyrZ;

  lastSent = millis();
  argument = millis() / 1000.0;
  sampleDataStr = "t:" + String(argument, 3) + "s";
  Serial.println(sampleDataStr);
  accX = sin(argument);
  accY = cos(argument);
  accZ = sin(argument) + cos(argument);
  sampleDataStr = "acc:"+String(accX,3)+","+String(accY,3)+","+String(accZ,3);
  Serial.println(sampleDataStr);
  gyrX = sin(argument);
  gyrY = cos(argument);
  gyrZ = sin(argument) + cos(argument);
  sampleDataStr = "gyr:"+String(gyrX,3)+","+String(gyrY,3)+","+String(gyrZ,3);
  Serial.println(sampleDataStr);  
}

void readServo() {
  int i, s[4], inChar;
  String input;
  input = "";
  i = 0;
  delay(50); //rewrite to read string and prase function
  //expected format: "000,000,000,000\n"
  while (Serial.available() > 0) {
    inChar = Serial.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      input += (char)inChar;
    }
    // if non-digit, save to s
    else if (inChar == ',') {
      s[i] = input.toInt();
      input = "";
      //Serial.print("added:");
      //Serial.println(s[i]);
      i++;
    }
    else if (inChar == '\n') {
      s[i] = input.toInt();
      input = "";
      Serial.print("New servo positions: ");
      for (i=0;i<=3;i++){
        Serial.print(String(s[i])+",");
        servoPos[i] = s[i];  
      }
      setServos(); //Set servo positions with servoPos[]
      Serial.println();      
    }
    else {
      Serial.println("Servo input error!");
    }
  }
}

void setServos() {
  servo1.write(servoPos[0]);
  servo2.write(servoPos[1]);
  servo3.write(servoPos[2]);
  servo4.write(servoPos[3]);
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  activateStream = false;
  Serial.setTimeout(100); //timeout before Serial.read function stops waiting for incoming data
  
  servo1.attach(servoPins[0]);
  servo2.attach(servoPins[1]);
  servo3.attach(servoPins[2]);
  servo4.attach(servoPins[3]);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {

    
    prijate = Serial.readStringUntil('\n'); // maybe cange to separate function with char adding to String
    prijate.trim(); //clear white spaces
    if (prijate == "h") {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (prijate == "l") {
      digitalWrite(LED_BUILTIN, LOW);
    }
    else if (prijate == "a") {
      activateStream = true;
    }
    else if (prijate == "t") {
      activateStream = false;
    }
    else if (prijate == "ser") {
    readServo();
  }    
    Serial.println(prijate);
  } 

   
  if (activateStream && ((millis() - lastSent) >= 10)) { //send every 10 milliseconds
    sendData();
  }

}
