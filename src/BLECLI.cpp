#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
//#include <esp32-hal-log.h>
//#include <U8g2lib.h>

//Declaration
void ble_client_init();
void ble_client_handle();

// The remote service NAME we wish to connect to.
static const char* sens1_name = "ADC-SENSOR#1";
static const char* sens2_name = "ADC-SENSOR#2";
static const char* sens3_name = "ADC-SENSOR#3";
static const char* sens4_name = "ADC-SENSOR#4";

//static BLEAddress sens1_address("ec:da:3b:be:25:16");//ADC-SENSOR#1
//static BLEAddress sens2_address("34:b7:da:f8:4c:b2");//ADC-SENSOR#2

//these UIDs are common to all sensors 
static BLEUUID serviceUUID("450475bb-56a2-4c97-9973-301831e65a30");
static BLEUUID    charUUID("d8182a40-7316-4cbf-9c6e-be507a76d775");

static boolean doConnect1 = false; //запрос на коннект с сенсором #1
static boolean connected1 = false; //текущее состояние связи #1
static boolean doConnect2 = false; //запрос на коннект с сенсором #2
static boolean connected2 = false; //текущее состояние связи #2
static boolean doConnect3 = false; //запрос на коннект с сенсором #3
static boolean connected3 = false; //текущее состояние связи #3
static boolean doConnect4 = false; //запрос на коннект с сенсором #4
static boolean connected4 = false; //текущее состояние связи #4
static boolean doScan = false;    //запрос на сканирования BLE сети

//живыe обьекты после коннекта
static BLERemoteCharacteristic* pRemoteCharacteristic1;
static BLERemoteCharacteristic* pRemoteCharacteristic2;
static BLERemoteCharacteristic* pRemoteCharacteristic3;
static BLERemoteCharacteristic* pRemoteCharacteristic4;
//живыe обьекты после сканирования
static BLEAdvertisedDevice* myDevice1;
static BLEAdvertisedDevice* myDevice2;
static BLEAdvertisedDevice* myDevice3;
static BLEAdvertisedDevice* myDevice4;

unsigned long previousMillis = 0;
unsigned long interval = 30000;  //30 sec. интервал между сканированием

//обработка события от характеристики, когда пришли данные с сервера BLE
static void notifyCallback1(
  BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    //печатаем полученную строку
    Serial.print("v1=");
    for (int i = 0; i < length; i++) {
      Serial.print((char)pData[i]);
    }
}
static void notifyCallback2(
  BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    //печатаем полученную строку
    Serial.print("v2=");
    for (int i = 0; i < length; i++) {
      Serial.print((char)pData[i]);
    }
}
static void notifyCallback3(
  BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    //печатаем полученную строку
    Serial.print("v3=");
    for (int i = 0; i < length; i++) {
      Serial.print((char)pData[i]);
    }
}
static void notifyCallback4(
  BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    //печатаем полученную строку
    Serial.print("v4=");
    for (int i = 0; i < length; i++) {
      Serial.print((char)pData[i]);
    }
}

//обработка события от нашего клиента1 BLE, connect-disconnect 1111111111111
class MyClientCallback1 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {  //сервер отпал..
    connected1 = false;
    Serial.println("Event-Disconnect1..");
    delay(100);
  }
};
//обработка события от нашего клиента2 BLE, connect-disconnect 2222222222222222
class MyClientCallback2 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {  //сервер отпал..
    connected2 = false;
    Serial.println("Event-Disconnect2..");
    delay(100);
  }
};
//обработка события от нашего клиента3 BLE, connect-disconnect 333333333333333
class MyClientCallback3 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {  //сервер отпал..
    connected3 = false;
    Serial.println("Event-Disconnect3..");
    delay(100);
  }
};
//обработка события от нашего клиента4 BLE, connect-disconnect 444444444444444
class MyClientCallback4 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {  //сервер отпал..
    connected4 = false;
    Serial.println("Event-Disconnect4..");
    delay(100);
  }
};

//подключение к серверу ********************************************************
bool connectToServer1() {

    doConnect1 = false; //однократно !!!!!!
    Serial.println("-Start connection our device 1..");
    
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback1());//connect-disconnect events !!!!!

    // Connect to the remove BLE Server.(это блокирующий вызов)
    boolean  r_ok = pClient->connect(myDevice1/*sens1_address*/);// адрес !!!!!!!
    if(!r_ok) return false; 

    // ссылка на удаленный сервис
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);

    if (pRemoteService == nullptr) {
      Serial.print("-Failed to find our service1 UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к сервису..
    }
    Serial.print("-Found our service1: ");
    Serial.println(serviceUUID.toString().c_str());

    // ссылка на удаленную характеристику..
    pRemoteCharacteristic1 = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic1 == nullptr) {
      Serial.print("-Failed to find our characteristic1 UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к характеристике..
    }
    Serial.print("-Found our characteristic1: ");
    Serial.println(pRemoteCharacteristic1->getUUID().toString().c_str());

    // событие порождается сервером...
    if(pRemoteCharacteristic1->canNotify()) 
        pRemoteCharacteristic1->registerForNotify(notifyCallback1,false);//!!!!!!

    return true;
}
bool connectToServer2() {

    doConnect2 = false; //однократно !!!!!!!
    Serial.println("-Start connection our device 2..");
    
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback2());//connect-disconnect events !!!!!

    // Connect to the remove BLE Server.(это блокирующий вызов)
    boolean  r_ok = pClient->connect(myDevice2/*sens2_address*/);// адрес !!!!!!!
    if(!r_ok) return false; 

    // ссылка на удаленный сервис
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);

    if (pRemoteService == nullptr) {
      Serial.print("-Failed to find our service2 UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к сервису..
    }
    Serial.print("-Found our service2: ");
    Serial.println(serviceUUID.toString().c_str());

    // ссылка на удаленную характеристику..
    pRemoteCharacteristic2 = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic2 == nullptr) {
      Serial.print("-Failed to find our characteristic2 UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к характеристике..
    }
    Serial.print("-Found our characteristic2: ");
    Serial.println(pRemoteCharacteristic2->getUUID().toString().c_str());

    // это терминальное свойство !!!
    // событие порождается сервером...
    if(pRemoteCharacteristic2->canNotify()) 
        pRemoteCharacteristic2->registerForNotify(notifyCallback2,false);//!!!!!!!!!!

    return true;
}
bool connectToServer3() {

    doConnect3 = false; //однократно !!!!!!
    Serial.println("-Start connection our device 3..");
    
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback3());//connect-disconnect events !!!!!

    // Connect to the remove BLE Server.(это блокирующий вызов)
    boolean  r_ok = pClient->connect(myDevice3/*sens3_address*/);// адрес !!!!!!!
    if(!r_ok) return false; 

    // ссылка на удаленный сервис
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);

    if (pRemoteService == nullptr) {
      Serial.print("-Failed to find our service3 UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к сервису..
    }
    Serial.print("-Found our service3: ");
    Serial.println(serviceUUID.toString().c_str());

    // ссылка на удаленную характеристику..
    pRemoteCharacteristic3 = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic3 == nullptr) {
      Serial.print("-Failed to find our characteristic3 UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к характеристике..
    }
    Serial.print("-Found our characteristic3: ");
    Serial.println(pRemoteCharacteristic3->getUUID().toString().c_str());

    // событие порождается сервером...
    if(pRemoteCharacteristic3->canNotify()) 
        pRemoteCharacteristic3->registerForNotify(notifyCallback3,false);//!!!!!!

    return true;
}
bool connectToServer4() {

    doConnect4 = false; //однократно !!!!!!!
    Serial.println("-Start connection our device 4..");
    
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback4());//connect-disconnect events !!!!!

    // Connect to the remove BLE Server.(это блокирующий вызов)
    boolean  r_ok = pClient->connect(myDevice4/*sens4_address*/);// адрес !!!!!!!
    if(!r_ok) return false; 

    // ссылка на удаленный сервис
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);

    if (pRemoteService == nullptr) {
      Serial.print("-Failed to find our service4 UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к сервису..
    }
    Serial.print("-Found our service4: ");
    Serial.println(serviceUUID.toString().c_str());

    // ссылка на удаленную характеристику..
    pRemoteCharacteristic4 = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic4 == nullptr) {
      Serial.print("-Failed to find our characteristic4 UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к характеристике..
    }
    Serial.print("-Found our characteristic4: ");
    Serial.println(pRemoteCharacteristic2->getUUID().toString().c_str());

    // это терминальное свойство !!!
    // событие порождается сервером...
    if(pRemoteCharacteristic4->canNotify()) 
        pRemoteCharacteristic4->registerForNotify(notifyCallback4,false);//!!!!!!!!!!

    return true;
}

//----------------------------------------------------------------
// событие, когда сканер находит очередной BLE сенсор
// если это один из наших, для него дается команда на подключение 
// и сохраняется его адрес
//----------------------------------------------------------------
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  // событие вызывается для каждого сервера рекламирующего услуги..
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    
    Serial.print("Scaner: Name=");Serial.print(advertisedDevice.getName().c_str());
    Serial.print(" Address=");Serial.print(advertisedDevice.getAddress().toString().c_str());

      // проверяем имена наших сенсоров
      if(advertisedDevice.haveName() && advertisedDevice.getName()==sens1_name/*"ADC-SENSOR#1"*/){
          myDevice1 = new BLEAdvertisedDevice(advertisedDevice); //class with address to connect
          doConnect1 = true; // команда: Connect
          Serial.println("  -Found..");
      } else if (advertisedDevice.haveName() && advertisedDevice.getName()==sens2_name/*"ADC-SENSOR#2"*/){
          myDevice2 = new BLEAdvertisedDevice(advertisedDevice); //class with address to connect
          doConnect2 = true; // команда: Connect
          Serial.println("  -Found..");
      } else if (advertisedDevice.haveName() && advertisedDevice.getName()==sens3_name/*"ADC-SENSOR#3"*/){
          myDevice3 = new BLEAdvertisedDevice(advertisedDevice); //class with address to connect
          doConnect3 = true; // команда: Connect
          Serial.println("  -Found..");
      } else if (advertisedDevice.haveName() && advertisedDevice.getName()==sens4_name/*"ADC-SENSOR#4"*/){
          myDevice4 = new BLEAdvertisedDevice(advertisedDevice); //class with address to connect
          doConnect4 = true; // команда: Connect
          Serial.println("  -Found..");
      } else {Serial.println();}
  } // onResult
}; // MyAdvertisedDeviceCallbacks

//--------------------------------------------------------------------------------------------------
//запускается после старта один раз в процедуре setup()
void ble_client_init(){

  BLEDevice::init("");

  //инициализируем процедуру сканирования сети  
  BLEScan* pBLEScan = BLEDevice::getScan(); //object singleton !
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);//интервал сканирования 1,3 сек
  pBLEScan->setWindow(449);//окно сканирования 449 мс
  pBLEScan->setActiveScan(true);

    doConnect1 = false;
    connected1 = false;
    doConnect2 = false;
    connected2 = false;
    doConnect3 = false;
    connected3 = false;
    doConnect4 = false;
    connected4 = false;
    doScan = true; //запрос на сканирование

    Serial.println("Go to Loop..");
}

//вызывается в цикле loop() =============================================================================
void ble_client_handle(){

  // периодически запускаем сканирование раз в [interval]
  // если есть хоть один неподключенный датчик 
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >=interval) {
    if(!connected1 || !connected2 || !connected3 || !connected4){ //хотя бы один отпал..
      doScan = true;
      Serial.println("Scanning Go..");
    }
    previousMillis = currentMillis;
  }

  //запрос - сканировать
  if(doScan){
    BLEDevice::getScan()->start(10/*0*/);//0-сканирует, пока принудительно не остановим.
    doScan = false; // однократно 
  }

  //получен зарос на CONNECT 1
  if (doConnect1) {
    if (connectToServer1()) { //подключение к серверу удачно
      Serial.println("We are now connected to the BLE Server1 - from loop()...");
      connected1 = true;
    } else {
      Serial.println("-Failed to connect our device 1 - from loop()...");
      connected1 = false;
    }
  }
  //получен зарос на CONNECT 2
  if (doConnect2) {
    if (connectToServer2()) { //подключение к серверу удачно
      Serial.println("We are now connected to the BLE Server2 - from loop()...");
      connected2 = true;
    } else {
      Serial.println("-Failed to connect our device 2 - from loop()...");
      connected2 = false;
    }
  }
  //получен зарос на CONNECT 3
  if (doConnect3) {
    if (connectToServer3()) { //подключение к серверу удачно
      Serial.println("We are now connected to the BLE Server3 - from loop()...");
      connected3 = true;
    } else {
      Serial.println("-Failed to connect our device 3 - from loop()...");
      connected3 = false;
    }
  }
  //получен зарос на CONNECT 4
  if (doConnect4) {
    if (connectToServer4()) { //подключение к серверу удачно
      Serial.println("We are now connected to the BLE Server4 - from loop()...");
      connected4 = true;
    } else {
      Serial.println("-Failed to connect our device 4 - from loop()...");
      connected4 = false;
    }
  }

  if (connected1) {
  // периодически посылаем команду серверу 1
    String newValue = "atv\r\n";
    pRemoteCharacteristic1->writeValue(newValue.c_str(), newValue.length());
    delay(50);
  }
  if (connected2) {
    // периодически посылаем команду серверу 2
    String newValue = "atv\r\n";
    pRemoteCharacteristic2->writeValue(newValue.c_str(), newValue.length());
    delay(50);
  }
  if (connected3) {
  // периодически посылаем команду серверу 3
    String newValue = "atv\r\n";
    pRemoteCharacteristic3->writeValue(newValue.c_str(), newValue.length());
    delay(50);
  }
  if (connected4) {
    // периодически посылаем команду серверу 4
    String newValue = "atv\r\n";
    pRemoteCharacteristic4->writeValue(newValue.c_str(), newValue.length());
    delay(50);
  }
    
  //Serial.print("connected="+String(connected));
  //Serial.print("  doScan="+String(doScan));
  //Serial.println("  doConnect="+String(doConnect));
}
