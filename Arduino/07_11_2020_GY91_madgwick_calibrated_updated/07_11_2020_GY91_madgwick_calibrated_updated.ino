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

#include <MPU9250.h> //https://github.com/bolderflight/MPU9250
#include "src\MadgwickAHRS.h" //https://github.com/arduino-libraries/MadgwickAHRS/blob/master/examples/Visualize101/Visualize101.ino
// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
//======variables======
MPU9250 IMU(Wire,0x68);
int status;
int i;
String received;
boolean activateStream = true;
//MAdgwick filter variables
Madgwick filter;
int millisPerReading, millisPrevious, millisNow;
int samplingFrequency;
float ax, ay, az;
float gx, gy, gz;
float mx, my, mz;
float roll, pitch, heading;
float t = 0.0;

int samplingFrequencyDef = 50; //Hz
//======functions=======

void updateData() {
  t = millis() / 1000.0;
  IMU.readSensor();
  millisPrevious = millisPrevious + millisPerReading;  
  
  ax = IMU.getAccelX_mss();
  ay = IMU.getAccelY_mss();
  az = IMU.getAccelZ_mss();

  //gx = IMU.getGyroX_rads()*180.0/PI; //to degrees per second
  //gy = IMU.getGyroY_rads()*180.0/PI;
  //gz = IMU.getGyroZ_rads()*180.0/PI;

  gx = IMU.getGyroX_rads(); 
  gy = IMU.getGyroY_rads();
  gz = IMU.getGyroZ_rads();

  mx = IMU.getMagY_uT(); //because of different magnetometer orientation
  my = IMU.getMagX_uT();
  mz = -IMU.getMagZ_uT();
  
  //filter input data
  filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
  //filter.updateIMU(gx/2, gy/2, gz/2, ax, ay, az); //WHY?!!!!!
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
//  Serial.print(dataStr + ";\t");
  dataStr = "hdg, pit, rol: " +
            String(heading,2) + "," +
            String(pitch,2) + "," +
            String(roll,2);
  Serial.println(dataStr);
}
void sensorsConfig() { //based on previous mesurements
  //configure accelerometer
  IMU.setAccelCalX(0.1848, 0.9975);
  IMU.setAccelCalY(-0.0638, 0.9975);
  IMU.setAccelCalZ(0.1746, 0.9923);
  //conf. magnetometer
  IMU.setMagCalX(13.376, 2.59);
  IMU.setMagCalY(29.16, 1.1952);
  //IMU.setMagCalZ(-63.9933, 0.5627);
  
  Serial.println("Press any key to continue");

}
//======setup======
void setup() {
  Wire.setClock(400000);
  
  samplingFrequency = samplingFrequencyDef;
  millisPerReading = 1000/samplingFrequency;
  
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
  status = IMU.setSrd(19); //
  Serial.println("SRD status code:" + String(status));

  sensorsConfig();
  Serial.print("Gyro bias rad/s: ");
  Serial.print(IMU.getGyroBiasX_rads(), 4);
  Serial.print("\t");
  Serial.print(IMU.getGyroBiasY_rads(), 4);
  Serial.print("\t");
  Serial.println(IMU.getGyroBiasZ_rads(), 4); 

  Serial.print("Accel scale Factor: ");
  Serial.print(IMU.getAccelScaleFactorX(), 4);
  Serial.print("\t\t");
  Serial.print(IMU.getAccelScaleFactorY(), 4);
  Serial.print("\t\t");
  Serial.println(IMU.getAccelScaleFactorZ(), 4);

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
