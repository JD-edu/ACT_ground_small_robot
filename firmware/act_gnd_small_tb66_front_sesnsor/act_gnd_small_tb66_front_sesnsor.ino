// install TB6612FNG library
//https://github.com/vincasmiliunas/ESP32-Arduino-TB6612FNG

#include <TB6612FNG.h>
#include "Adafruit_VL53L0X.h"
#include <MPU6050_light.h>
#include "Wire.h"
//#include <U8x8lib.h>
#include "BluetoothSerial.h"
#include <SPI.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>

MPU6050 mpu(Wire);

#define MSLEEP  16
#define IA1     14
#define IB1     12
#define PWM1    13
#define IA2     27
#define IB2     26
#define PWM2    25
#define IA3     17
#define IB3     5
#define PWM3    18
#define IA4     4
#define IB4     2
#define PWM4    15

#define CONNECTED   23

#define WIFI_SSID  "jdedu9807"
#define WIFI_PASSWD "jdedu9807"

String macAddress = "";
String ipAddress = "";

//const char *websockets_server_host = "cobot.center";
//const uint16_t websockets_server_port = 8286;
const char *websockets_server_host = "192.168.0.77";
const uint16_t websockets_server_port = 8276;
using namespace websockets;
WebsocketsClient client;

Tb6612fng motors1(16, 14, 12, 13, 26, 27, 25);
Tb6612fng motors2(16, 5, 17, 18, 2, 4, 15);

// set the pins to shutdown
#define SHT_LOX1 32
#define SHT_LOX2 33
#define SHT_LOX3 3
#define SHT_LOX4 19

#define LOX1_ADDRESS 0x40
#define LOX2_ADDRESS 0x41
#define LOX3_ADDRESS 0x42
#define LOX4_ADDRESS 0x43

#define speed 0.7

// Sensor variables
int sensor1,sensor2, sensor3, sensor4;
int current_angle, start_angle, end_angle;

int beat_count = 0;
int turnState = 0;

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox4 = Adafruit_VL53L0X();

unsigned long timer = 0;

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;
VL53L0X_RangingMeasurementData_t measure3;
VL53L0X_RangingMeasurementData_t measure4;

void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Got Message: ");
  if (message.isText())
  {
    const char *command = message.c_str();
    //Serial.println(command);
    if (strcmp(command, "AU") == 0){         
      go_forward();
    }
    if (strcmp(command, "AD") == 0){  
      go_backward();
    }
    if (strncmp(command, "AL", 2) == 0){
      turn_left();
    }
    if (strncmp(command, "AR", 2) == 0){
      turn_right();
    }
    if (strcmp(command, "STOP") == 0){
      stop();
    }
  }
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if(event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
    digitalWrite(CONNECTED, HIGH);
  } else if(event == WebsocketsEvent::ConnectionClosed) {
    digitalWrite(CONNECTED, LOW);
    delay(100);
    Serial.println("Connnection Closed... Rebooting");
    ESP.restart();
  } else if(event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
  } else if(event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

void read_dual_sensors() {
  
  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  //lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!
  //lox3.rangingTest(&measure3, false); // pass in 'true' to get debug data printout!
  //lox4.rangingTest(&measure4, false); // pass in 'true' to get debug data printout!

  // print sensor one reading
  if(measure1.RangeStatus != 4) {     // if not out of range
    sensor1 = measure1.RangeMilliMeter;    
  } else {
    //Serial.print("Out of range");
  }
  // print sensor two reading
  //if(measure2.RangeStatus != 4) {
  //  sensor2 = measure2.RangeMilliMeter;
  //} else {
    //Serial.print("Out of range");
  //}
  // print sensor three reading
  //if(measure3.RangeStatus != 4) {
  //  sensor3 = measure3.RangeMilliMeter;
  //} else {
    //  Serial.print("Out of range");
  //}
  // print sensor four reading
  //if(measure3.RangeStatus != 4) {
  //  sensor4 = measure4.RangeMilliMeter;
  //} else {
    //  Serial.print("Out of range");
  //}
}

void setID() {
  // all reset
  digitalWrite(SHT_LOX1, LOW);    
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);
  delay(10);
  // all unreset
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, HIGH);
  digitalWrite(SHT_LOX3, HIGH);
  digitalWrite(SHT_LOX4, HIGH);
  //delay(10);

  // activating LOX1 and reseting LOX2
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);

  // initing LOX1
  if(!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while(1){
      if(!lox1.begin(LOX1_ADDRESS)){
        break;
      }
    }
  }
  delay(10);
  /*
  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  //initing LOX2
  if(!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while(1){
      if(!lox2.begin(LOX2_ADDRESS)){
        break;
      }
    }
  }

  // activating LOX3
  digitalWrite(SHT_LOX3, HIGH);
  delay(10);

  //initing LOX3
  if(!lox3.begin(LOX3_ADDRESS)) {
    Serial.println(F("Failed to boot third VL53L0X"));
    while(1){
       if(!lox3.begin(LOX3_ADDRESS)){
        break;
      }
    };
  }

  // activating LOX4
  digitalWrite(SHT_LOX4, HIGH);
  delay(10);
  //initing LOX4
  if(!lox4.begin(LOX4_ADDRESS)) {
    Serial.println(F("Failed to boot third VL53L0X"));
    while(1){
      if(!lox3.begin(LOX3_ADDRESS)){
        break;
      }
    };
  }*/
}

void go_forward(){
  Serial.println("forward");
  motors1.drive(-speed);
  motors2.drive(-speed);
}

void go_backward(){
  Serial.println("backward");
  motors1.drive(speed);
  motors2.drive(speed);
}

void turn_right(){
  Serial.println("right");
  motors1.drive(-speed);
  motors2.drive(speed);
}

void turn_left(){
   Serial.println("left");
  motors1.drive(speed);
  motors2.drive(-speed);
}

void stop(){
  Serial.println("stop");
  motors1.drive(0);
  motors2.drive(0);
  delay(200);
}


void setup() {
  Serial.begin(115200);
  Serial.print("ACT ground robot Start...");

  motors1.begin();
  motors2.begin();

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  pinMode(SHT_LOX3, OUTPUT);
  pinMode(SHT_LOX4, OUTPUT);

  pinMode(CONNECTED, OUTPUT);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected.");

  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);
  // c3rl3c86n88jq9lrl3gg LegoMars-1
  // c3rl3f86n88jq9lrl3hg LegoMars-2
  // c3rl3jg6n88jq9lrl3ig LegoMars-3
  // c3rl3l06n88jq9lrl3jg LegoMars-4
  // c3rl3to6n88jq9lrl3lg LegoMars-5
  // c3rl4006n88jq9lrl3mg LegoMars-6
  // c3rl4286n88jq9lrl3ng LegoMars-7
  // c3rl43o6n88jq9lrl3og LegoMars-8
  // c3rl4586n88jq9lrl3pg LegoMars-9
  // cakjdd4k058s72qr0prg MarsCamera-1
  // cakjdgsk058s72qr0psg MarsCamera-2
  // cakjdjck058s72qr0ptg MarsCamera-3rsCAmera-4
  while (!client.connect(websockets_server_host, websockets_server_port, "/pang/ws/pub?channel=c3rl43o6n88jq9lrl3og&track=colink&mode=bundle"))
  {
      delay(500);
      Serial.print(".");
  }
  
  Serial.println("Websocket Connected.");
  Wire.begin();

  delay(500);
  
  /*byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ 
    delay(50);
    byte status = mpu.begin();
    Serial.print(F("MPU6050 status: "));
    Serial.println(status);
    if(status == 0)
      break;
  } // stop everything if could not connect to MPU6050
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");
  delay(1000);*/
  Serial.println("VL53L0X: setup...");
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);

  Serial.println("VL53L0X: Both in reset mode...(pins are low)");
  Serial.println("VL53L0X: Starting...");
  setID();
  delay(500);
  
}


void loop() {
  if(Serial.available() > 0){
    char c = Serial.read();
    Serial.println(c);
      if(c == 'f'){
        go_forward();
    }else if(c == 'b'){
        go_backward();
    }else if(c == 'l'){
        turn_left();
    }else if(c == 'r'){
        turn_right(); 
    }else if(c == 'p'){
        Serial.println("stop");
        stop();
    }else if(c == 'h'){

    }
  }

  if((millis()-timer)>50){ // print data every 10ms
    //mpu.update();
    read_dual_sensors();
    //current_angle = mpu.getAngleZ();
	  timer = millis();
  }
    
  client.poll();
  beat_count++;
  if(beat_count > 50){
    digitalWrite(CONNECTED, !digitalRead(CONNECTED));
    String data = 'a'+String(current_angle)+'b'+String(sensor1)+'c'+String(sensor2)+'d'+String(sensor3)+'e'+String(sensor4)+'f';
    client.send(data);
    beat_count = 0;
    Serial.print(current_angle);
    Serial.print(' ');
    Serial.print(sensor1);
    Serial.print(' ');
    Serial.print(sensor2);
    Serial.print(' ');
    Serial.print(sensor3);
    Serial.print(' ');
    Serial.println(sensor4);
  }
}
