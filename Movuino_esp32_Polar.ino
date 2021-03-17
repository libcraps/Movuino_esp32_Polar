#include "BLEDevice.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOSC.h>
#include "Wire.h"
//#include "MPU6050.h"
//#include "I2Cdev.h"
#include <MPU9250_asukiaaa.h>

//Bluetooth
BLEUUID serviceUUID(BLEUUID((uint16_t)0x180D)); // remote service
BLEUUID charUUID(BLEUUID((uint16_t)0x2A37)); // Caract of remote service
BLEAddress *pServerAd;
boolean doConnectBT = false;
boolean connectedBT = false;
BLERemoteCharacteristic* pCaract;

//Wifi
char* ssid = "GHG9D7511A0001 2565"; //Network SSID
char* password = "A!26p741"; //Network password
char * hostIP =  "192.168.137.1";       // IP address of the host computer
const int send_port = 55555;
IPAddress remoteIP(192, 168, 137, 190); // remote IP to send UDP packet to
unsigned int remotePort = 2390; // remote port to send UDP packet to
const unsigned int portOut = 7400;// port on which data are sent (send OSC message)

WiFiUDP Udp;

//MPU
MPU9250_asukiaaa mpu9250;
//MPU6050 mpu6050;
int packetNumber = 0;
int16_t ax, ay, az; // store accelerometre values
int16_t gx, gy, gz; // store gyroscope values
int16_t mx, my, mz; // store magneto values
int magRange[] = {666, -666, 666, -666, 666, -666}; // magneto range values for callibration

// LEDs
const int pinLedWifi = 2; // wifi led indicator
const int pinLedBat = 0;  // battery led indicator

class advDevCallBacks: public BLEAdvertisedDeviceCallbacks 
{
  
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advDev) 
    {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advDev.toString().c_str());
      
      if (advDev.haveServiceUUID() && advDev.getServiceUUID().equals(serviceUUID))  // We have found a device, let us now see if it contains the service we are looking for.
      {
        //
        Serial.print("Found our device!  address: ");
        advDev.getScan()->stop();
        pServerAd = new BLEAddress(advDev.getAddress());
        doConnectBT = true;
      }
    }
    
};

void setup() 
{
  //Wire.begin();
  //mpu9250.setWire(&Wire);
  Serial.begin(115200);
  //mpu6050.initialize();
  //Serial.println("Initializing I2C devices...");
  //mpu6050.initialize();
  //mpu9250.beginAccel();
  
  Serial.print("Connecting to WIFI");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting connecting BLE...");
  
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new advDevCallBacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() 
{
  // connect then we scan to found the desired BLE Server
  if (doConnectBT == true) {
    if (connectToBT(*pServerAd)) {
      Serial.println("BLE Server connected");
      connectedBT = true;
    } else {
      Serial.println("connect to the server failure");
    }
    doConnectBT = false;
  }
  OscWiFi.update(); // should be called to receive + send osc
//  mpu6050.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz); // Get all 9 axis data (acc + gyro + magneto)
//  Serial.println(" ");
//  Serial.println(ax);
  delay(1000);
}
