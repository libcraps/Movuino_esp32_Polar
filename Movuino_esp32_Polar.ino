#include "BLEDevice.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>        
#include <Ethernet.h>
// The remote service we wish to connect to.
static  BLEUUID serviceUUID(BLEUUID((uint16_t)0x180D));
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID(BLEUUID((uint16_t)0x2A37));
static BLEAddress *pServerAddress;
static boolean doConnectBT = false;
static boolean connectedBT = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static const char* ssid     = "OnePlus_SK";       //Network SSID
static const char* password = "Salutmongars";     //Network password
IPAddress remoteIP(192, 168, 43, 168);      // remote IP to send UDP packet to
unsigned int remotePort = 2390;      // remote port to send UDP packet to
char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back
WiFiUDP Udp;
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
//  Serial.print("Notify callback for characteristic ");
  Udp.beginPacket(remoteIP, remotePort);
  for (int i = 0; i < length; i++) {
    Udp.write(&pData[i], sizeof(uint8_t));
    Serial.print(pData[i]);
    Serial.print(" ");
  }
  Udp.endPacket();
  Serial.println();
}
bool connectToBT(BLEAddress pAddress) {
  Serial.print("Forming a connection to ");
  Serial.println(pAddress.toString().c_str());
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  // Connect to the remove BLE Server.
  pClient->connect(pAddress, (esp_ble_addr_type_t)1);
//  pClient->connect(pAddress, BLE_ADDR_TYPE_RANDOM); //  Should work as well - BLE_ADDR_TYPE_RANDOM is equal 1 in enumerator definition
  Serial.println(" - Connected to server");
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our service");
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our characteristic");
  // Read the value of the characteristic.
  std::string value = pRemoteCharacteristic->readValue();
  Serial.print("The characteristic value was: ");
  Serial.println(value.c_str());
  pRemoteCharacteristic->registerForNotify(notifyCallback);
  const uint8_t notificationOn[] = {0x1, 0x0};
  pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
  return true;
}
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
        //
        Serial.print("Found our device!  address: ");
        advertisedDevice.getScan()->stop();
        pServerAddress = new BLEAddress(advertisedDevice.getAddress());
        doConnectBT = true;
      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks
void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
} // End of setup.
void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnectBT == true) {
    if (connectToBT(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connectedBT = true;
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnectBT = false;
  }
  delay(1000); // Delay a second between loops.
} // End of loop