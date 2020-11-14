/*
Basic_I2C.ino
Brian R Taylor
brian.taylor@bolderflight.com
Copyright (c) 2017 Bolder Flight Systems
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, 
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or 
substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* This sketch is designated to configure MPU9250 mesurements for future uses. Madgwick filter is
 * included as well to check calibration rightaway.
 * To start Accel confihuration type 'confA'
 * To start magnetometer configuration type 'confM'
 * In order to configure accMeter, you have to mesure gravitational acceleration at 6 different orientations,
 * i.e. with every axis facing once directly up or directly down.
 * In order configure magnetometer, mowe GY-91 slowly along 8-shaped figure
 */
#include <MPU9250.h> //https://github.com/bolderflight/MPU9250
#include "src\MadgwickAHRS.h" //https://github.com/arduino-libraries/MadgwickAHRS/blob/master/examples/Visualize101/Visualize101.ino
// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
//======variables======
MPU9250 IMU(Wire,0x68);
int status;
int i;
String received;
boolean activateStream = false;
//MAdgwick filter variables
Madgwick filter;
int millisPerReading, millisPrevious, millisNow;
int samplingFrequency;
float ax, ay, az;
float gx, gy, gz;
float mx, my, mz;
float roll, pitch, heading;
float t = 0.0;

int samplingFreqencyDef = 50; //Hz
//======functions=======

void updateData() {
  t = millis() / 1000.0;
  IMU.readSensor();
  millisPrevious = millisPrevious + millisPerReading;  
  
  ax = IMU.getAccelX_mss();
  ay = IMU.getAccelY_mss();
  az = IMU.getAccelZ_mss();

  gx = IMU.getGyroX_rads();
  gy = IMU.getGyroY_rads();
  gz = IMU.getGyroZ_rads();

  mx = IMU.getMagY_uT(); //because of different magnetometer orientation, google datasheet for MPU9250
  my = IMU.getMagX_uT();
  mz = -IMU.getMagZ_uT();
  
  //filter input data
  filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
  //save orientation in space
  roll = filter.getRoll();
  pitch = filter.getPitch();
  heading = filter.getYaw();
}

void sendData() {
  String dataStr;
  dataStr = "t:" + String(t, 4) + "s";
  Serial.print(dataStr + "; ");
  dataStr = "acc:"+
            String(sqrt(ax*ax + ay*ay + az*az)) + ":"+
            String(ax,4) + ","+
            String(ay,4) + ","+
            String(az,4);
  Serial.print(dataStr + ";\t");
  dataStr = "gyr:"+
            String(gx,4) + ","+
            String(gy,4) + ","+
            String(gz,4);
  Serial.print(dataStr + ";\t");
//  dataStr = "mag:"+
//            String(mx,4) + ","+
//            String(my,4) + ","+
//            String(mz,4);
  Serial.print(dataStr + ";\t");
  dataStr = "hdg, pit, rol: " +
            String(heading,2) + "," +
            String(pitch,2) + "," +
            String(roll,2);
  Serial.println(dataStr);
}
void accelConfig() {
  boolean finished = false;
  i = 0;
  Serial.println("Press any key to continue, press 't' to terminate");
  while(!finished & (i<6)) {
    
    if (Serial.available() > 0) {    
      received = Serial.readStringUntil('\n');
      received.trim(); 
      if (received == "t") { //termination
        finished = true;
      }
      else {
        IMU.calibrateAccel();
        //show results afther every mesurement
        Serial.print("Accel bias, m/s^2: ");
        Serial.print(IMU.getAccelBiasX_mss(), 4);
        Serial.print("\t\t");
        Serial.print(IMU.getAccelBiasY_mss(), 4);
        Serial.print("\t\t");
        Serial.println(IMU.getAccelBiasZ_mss(), 4);

        Serial.print("Accel scale Factor: ");
        Serial.print(IMU.getAccelScaleFactorX(), 4);
        Serial.print("\t\t");
        Serial.print(IMU.getAccelScaleFactorY(), 4);
        Serial.print("\t\t");
        Serial.println(IMU.getAccelScaleFactorZ(), 4);
        i++;
        Serial.println("Press any key to continue");
      }      
    }
    else {
      delay(500);
    }
  }
}

void magConfig() {

  Serial.println("Calibrating magnetometer...");
  Serial.println("Move GY-91 slowly in a 8-shaped figure");
  status = IMU.calibrateMag();
  Serial.println("Status code:" + String(status));

  Serial.print("Magnetometer bias uT: ");
  Serial.print(IMU.getMagBiasX_uT(), 4);
  Serial.print("\t");
  Serial.print(IMU.getMagBiasY_uT(), 4);
  Serial.print("\t");
  Serial.println(IMU.getMagBiasZ_uT(), 4);

  Serial.print("Magnetometer scalefactor: ");
  Serial.print(IMU.getMagScaleFactorX(), 4);
  Serial.print("\t");
  Serial.print(IMU.getMagScaleFactorY(), 4);
  Serial.print("\t");
  Serial.println(IMU.getMagScaleFactorZ(), 4);
}
//======setup======
void setup() {
  Wire.setClock(400000);
  samplingFrequency = samplingFreqencyDef;
  millisPerReading = 1000/samplingFrequency; //i.e. 50 Hz
  
  // serial to display data
  Serial.begin(115200);
  while(!Serial) {}

  // start communication with IMU   
  status = IMU.begin();
  Serial.println("Status code:" + String(status));
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  IMU.setGyroRange(MPU9250::GYRO_RANGE_250DPS);
  status = IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  Serial.println("DLPF status code:" + String(status));  
  status = IMU.setSrd(19);
  Serial.println("SRD status code:" + String(status));
  
  Serial.print("Gyro bias rad/s: ");
  Serial.print(IMU.getGyroBiasX_rads(), 4);
  Serial.print("\t");
  Serial.print(IMU.getGyroBiasY_rads(), 4);
  Serial.print("\t");
  Serial.println(IMU.getGyroBiasZ_rads(), 4);

  millisPrevious = millis();  
  
}
//======main loop======
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
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (received == "l") { //check connection
      digitalWrite(LED_BUILTIN, LOW);
    }
    else if (received == "confA") {
      accelConfig();    
    }
    else if (received == "confM") {
      magConfig();    
    }
    //Serial.println(received); //for debugging purposes
  }

  if (millis() - millisPrevious >= millisPerReading) {
    // update
    updateData();
    // send data
    if (activateStream) {
        sendData();  
    }
  }
  
}
