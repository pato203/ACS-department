//MPU9250 module reading
#include <MPU9250.h> //https://github.com/bolderflight/MPU9250
TwoWire Wire2(2,I2C_FAST_MODE); //Wire2 becuse we have I2C wires connected to SDA2/SCL2
//I2S_FAST_MODE clock frequency 400 000 Hz

//Arduino default madgwick filter + AHRS
#include <MadgwickAHRS.h> //https://github.com/arduino-libraries/MadgwickAHRS/blob/master/examples/Visualize101/Visualize101.ino

//======variables=======
String received; 
boolean activateStream = true;
int timeDelta = 10; //ms
int lastSent=0;
// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
MPU9250 IMU(Wire2, 0x68);
int status;
//madgwick filter variables
Madgwick filter;
unsigned long microsPerReading, microsPrevious, microsNow;
int samplingFrequency;
float ax, ay, az;
float gx, gy, gz;
float roll, pitch, heading;
float t = 0.0;

//======functions=======

void updateData() {
  t = millis() / 1000.0;
  IMU.readSensor();
  ax = IMU.getAccelX_mss();
  ay = IMU.getAccelY_mss();
  az = IMU.getAccelZ_mss();

  gx = IMU.getGyroX_rads()*180.0/PI; //to degrees per second
  gy = IMU.getGyroY_rads()*180.0/PI;
  gz = IMU.getGyroZ_rads()*180.0/PI;
  //filter input data
  filter.updateIMU(gx, gy, gz, ax, ay, az);
  //save orientation in space
  roll = filter.getRoll();
  pitch = filter.getPitch();
  heading = filter.getYaw();

  microsPrevious = microsPrevious + microsPerReading;  
}

void sendData() {
  String dataStr;
  dataStr = "t:" + String(t, 4) + "s";
  Serial.println(dataStr);
  dataStr = "acc:"+
            String(ax,4) + ","+
            String(ay,4) + ","+
            String(az,4);
  Serial.println(dataStr);
  dataStr = "gyr:"+  // workaround to feed Euler angles data to computer GUI on gyr channel
            String(heading,4) + ","+
            String(pitch,4) + ","+
            String(roll,4);
  Serial.println(dataStr);
}

void setup() {
  
  Serial.begin(115200);
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
  status = IMU.setGyroRange(MPU9250::GYRO_RANGE_250DPS); //for better accuracy
  samplingFrequency = 1000 / timeDelta; //Hz
  microsPerReading = 1000000 / samplingFrequency; //us 
  microsPrevious = micros();

}

void loop() {
  
  if (Serial.available() > 0) {    
    received = Serial.readStringUntil('\n');
    received.trim(); //clear white spaces
    
    if (received == "a") {
      activateStream = true;
    }
    else if (received == "t") {
      activateStream = false;
    }
    else if (received == "h") { //check connection
      //digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (received == "l") { //check connection
      //digitalWrite(LED_BUILTIN, LOW);
    }
    else if (received == "ser") {
      Serial.println("No servo this time!");
    }    
    Serial.println(received); //for debugging purposes
  }

  if (activateStream && (micros() - microsPrevious >= microsPerReading)) {
    updateData();
    sendData();
  }

}
