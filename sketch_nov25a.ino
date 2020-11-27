#include <ESP32Servo.h>
#include <analogWrite.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

//BLE Parameter
#define SERVICE_UUID "ae975c80-05f0-11ea-8d71-362b9e155667"
#define COMMAND_CHAR_UUID "b81f189c-05f0-11ea-8d71-362b9e155667"
BLEDevice Device;
BLEServer* pServer;
BLEService* pService;
BLECharacteristic* pCharCommand;
BLEAdvertising* pAdvertising;

//Servo Parameter
Servo servo1;
int pos = 0;
static const int servoPin = 4;

//Motor Parameter
int motorDir = 18;
int motorPWM = 5; 
  
void setup() {
  
  Serial.begin(115200);
  pinMode(motorDir,OUTPUT);
  pinMode(motorPWM,OUTPUT);

  initBLE();

  //attach servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo1.setPeriodHertz(50);
  servo1.attach(servoPin,1000,2000);
  servo1.write(90);
  
  stopMotor();
  }

void loop() { 
}

void motorPower(int fb) { //value from 0 to 255, 128 to stop motor
  
    if(fb == 128)
      stopMotor();
    else if(fb > 128)
      moveForward(fb);
    else
      moveBackward(fb);
      
      }


      void stopMotor(){
  
    digitalWrite(motorDir,LOW);
    analogWrite(motorPWM,LOW);
    
    }

    void moveForward(int fb){
  
    int pow= 255-((fb-128)*2);
    digitalWrite(motorDir,HIGH);
    analogWrite(motorPWM,pow);
    
    }


    
void moveBackward(int fb){
  
    int pow= 255-(fb*2);
    digitalWrite(motorDir,LOW);
    analogWrite(motorPWM,pow);
    
    }
      


void carControll(int steerr, int pow){
  
    steer(steerr);
    motorPower(pow);
    
    }

void steer(int steerr){
  
  servo1.write(steerr);
 }

class ServerCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer* pServerCallback){
  Serial.println("Client connected");
  }
   void onDisconnect(BLEServer* pServerCallback){
  Serial.println("Client Disconnected");
  stopMotor();
  }
};

class CharacteristicCallbacks: public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic* pCharacteristic){
    std::string value = pCharacteristic->getValue();
    String svalue=value.c_str();

    int splitindex = svalue.indexOf(',');
    int speed = svalue.substring(0,splitindex).toInt();
    int steer = svalue.substring(splitindex+1).toInt();

    carControll(steer,speed);
   }
};



  void initBLE(){
  Device.init("Racer");
  pServer = Device.createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharCommand = pService->createCharacteristic(COMMAND_CHAR_UUID,
  BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  
  Device.setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  pCharCommand->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  pServer->setCallbacks(new ServerCallbacks());
  
  pCharCommand->setCallbacks(new CharacteristicCallbacks());
  pCharCommand->setValue("128,90");
pService->start();
  pAdvertising = Device.getAdvertising();
pAdvertising-> addServiceUUID(SERVICE_UUID);
pAdvertising->setScanResponse(true);
pAdvertising->setMinPreferred(0x06);
pAdvertising->setMinPreferred(0x12);
Device.startAdvertising();
Serial.println("Start Advertising");
  
  
  }
