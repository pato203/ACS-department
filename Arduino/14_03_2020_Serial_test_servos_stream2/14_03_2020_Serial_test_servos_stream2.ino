#include <Servo.h>
#include <MPU9250.h> //https://github.com/bolderflight/MPU9250
TwoWire Wire2(2,I2C_FAST_MODE); //clock frequency 400 000 Hz


String received; //i.e. received
//String sampleDataStr;
boolean activateStream = true;
int servoPos[] ={90,90,90,90} ;
int timeDelta = 10;
int lastSent=0;
// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
MPU9250 IMU(Wire2, 0x68);
int status;

//======functions=======

void sendMPU() {
  String dataStr = "";
  IMU.readSensor();
  dataStr = "acc:"+
            String(IMU.getAccelX_mss(),4) + ","+
            String(IMU.getAccelY_mss(),4) + ","+
            String(IMU.getAccelZ_mss(),4);
  Serial.println(dataStr);
  dataStr = "gyr:"+
            String(IMU.getGyroX_rads(),4) + ","+
            String(IMU.getGyroY_rads(),4) + ","+
            String(IMU.getGyroZ_rads(),4);
  Serial.println(dataStr);
}

void sendData() {
  float argument, accX, accY, accZ, gyrX, gyrY ,gyrZ, t;
  String dataStr;
  lastSent = millis();
  t = millis() / 1000.0;
  dataStr = "t:" + String(t, 4) + "s";
  Serial.println(dataStr);
  sendMPU();
  /*accX = sin(argument);
  accY = cos(argument);
  accZ = sin(argument) + cos(argument);
  sampleDataStr = "acc:"+String(accX,3)+","+String(accY,3)+","+String(accZ,3);
  Serial.println(sampleDataStr);
  gyrX = sin(argument);
  gyrY = cos(argument);
  gyrZ = sin(argument) + cos(argument);
  sampleDataStr = "gyr:"+String(gyrX,3)+","+String(gyrY,3)+","+String(gyrZ,3);
  Serial.println(sampleDataStr);*/  
}

int *parsePosition(String input, int ServArray[4]) {
  int j;
  int from = 0;
  char to = ',';
  int ind_del = input.indexOf(to);
  for (j=0; j<=2; j++) {
    ServArray[j] = input.substring(from, ind_del).toInt();
    input.remove(from, ind_del + 1);
    ind_del = input.indexOf(to);
  }
  ServArray[3] = input.substring(from, input.length()).toInt();
  return ServArray;
  
}

void readServo() {
  int i,ServArray[4], inChar;
  String input;
  input = "";
  i = 0;
  input = Serial.readStringUntil('\n');
  if(input=="")
     Serial.println("Error input");
  else {
    parsePosition(input, ServArray);
     for (i=0;i<=3;i++){
        Serial.print(String(ServArray[i])+",");
        servoPos[i] = ServArray[i];  
      }
     setServos();
  }
  //expected format: "000,000,000,000\n"
 
}



void setServos() {
 //delete later  
}

void setup() {
  Serial.begin(115200);
  //pinMode(LED_BUILTIN,OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW);
  activateStream = false;
  Serial.setTimeout(100); //timeout before Serial.read function stops waiting for incoming data

  // start communication with IMU 
  status = IMU.begin();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }
  

}

void loop() {
  
  if (Serial.available() > 0) {    
    received = Serial.readStringUntil('\n'); // maybe cange to separate function with char adding to String
    received.trim(); //clear white spaces
    if (received == "h") {
      //digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (received == "l") {
      //digitalWrite(LED_BUILTIN, LOW);
    }
    else if (received == "a") {
      activateStream = true;
    }
    else if (received == "t") {
      activateStream = false;
    }
    else if (received == "ser") {
    readServo();
  }    
    Serial.println(received);
  }

   
  if (activateStream && ((millis() - lastSent) >= timeDelta)) { //send every timeDelta milliseconds
    sendData();
  }

}
