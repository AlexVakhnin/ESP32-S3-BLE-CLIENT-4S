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

//static BLEAddress sens1_address("ec:da:3b:be:25:16");//ADC-SENSOR#1
//static BLEAddress sens2_address("34:b7:da:f8:4c:b2");//ADC-SENSOR#2

//these UIDs are common to all sensors 
static BLEUUID serviceUUID("450475bb-56a2-4c97-9973-301831e65a30");
static BLEUUID    charUUID("d8182a40-7316-4cbf-9c6e-be507a76d775");

static boolean doConnect1 = false; //запрос на коннект с сенсором #1
static boolean connected1 = false; //текущее состояние связи #1
static boolean doConnect2 = false; //запрос на коннект с сенсором #2
static boolean connected2 = false; //текущее состояние связи #2
static boolean doScan = false;    //запрос на сканирования BLE сети

//живыe обьекты после коннекта
static BLERemoteCharacteristic* pRemoteCharacteristic1;
static BLERemoteCharacteristic* pRemoteCharacteristic2;
//живыe обьекты после сканирования
static BLEAdvertisedDevice* myDevice1;
static BLEAdvertisedDevice* myDevice2;

unsigned long previousMillis = 0;
unsigned long interval = 25000;  //25 sec.

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

//обработка события от нашего клиента1 BLE, connect-disconnect 1111111111111
class MyClientCallback1 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {  //сервер отпал..
    connected1 = false;
    Serial.println("Event-Disconnect1..");
    delay(500);
  }
};
//обработка события от нашего клиента2 BLE, connect-disconnect 2222222222222222
class MyClientCallback2 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }
  void onDisconnect(BLEClient* pclient) {  //сервер отпал..
    connected2 = false;
    Serial.println("Event-Disconnect2..");
    delay(500);
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
      Serial.print("-Failed to find our service1 UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false; //не смогли подключиться к сервису..
    }
    Serial.print("-Found our service2: ");
    Serial.println(serviceUUID.toString().c_str());

    // ссылка на удаленную характеристику..
    pRemoteCharacteristic2 = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic2 == nullptr) {
      Serial.print("-Failed to find our characteristic1 UUID: ");
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
    doScan = true; //запрос на сканирование

    Serial.println("Go to Loop..");
}

//вызывается в цикле loop() =============================================================================
void ble_client_handle(){

  // периодически запускаем сканирование раз в [interval]
  // если есть хоть один неподключенный датчик 
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >=interval) {
    if(!connected1 || !connected2){
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
    
  //Serial.print("connected="+String(connected));
  //Serial.print("  doScan="+String(doScan));
  //Serial.println("  doConnect="+String(doConnect));
}
