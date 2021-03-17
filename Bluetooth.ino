static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharac, uint8_t* pData, size_t length, bool isNotify)
{
  /*
   * Function that is called each notification
   */
  Udp.beginPacket(remoteIP, remotePort);
  for (int i = 0; i < length; i++) 
  {
    Udp.write(&pData[i], sizeof(uint8_t));
    Serial.print(pData[i]);
    Serial.print(" ");
    
  }
  OscWiFi.send(hostIP, portOut, "/bpm", 1);
  Udp.endPacket();
  Serial.println();
}

bool connectToBT(BLEAddress pAddress) 
{
  Serial.print("Forming a connection to ");
  Serial.println(pAddress.toString().c_str());
  
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
 
  pClient->connect(pAddress, (esp_ble_addr_type_t)1);  // Connect to the remove BLE Server.
  Serial.println(" - Connected to server");
  
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID); // gettain a reference to the service
  
  if (pRemoteService == nullptr) 
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    
    return false;
  }
  Serial.println(" - Found service");
  
  pCaract = pRemoteService->getCharacteristic(charUUID); // get a reference to the characteristic
  
  if (pCaract == nullptr) 
  {
    Serial.print("Failed to find our charact UUID: ");
    Serial.println(charUUID.toString().c_str());
    
    return false;
  }
  Serial.println(" - Found our characteristic");
  
  std::string value = pCaract->readValue();
  Serial.print("Caract value : ");
  Serial.println(value.c_str());
  pCaract->registerForNotify(notifyCallback);
  uint8_t notifOn[] = {0x1, 0x0};
  pCaract->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notifOn, 2, true);
  
  return true;
}
