#include "Adafruit_VL53L0X.h"
#include <MPU6050_light.h>
#include "Wire.h"
#include "BluetoothSerial.h"
#include <SPI.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "MPU9250.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();\
MPU9250 mpu;

unsigned long timer = 0;

#define M1_B    26
#define M1_A    27
#define M2_B    12
#define M2_A    14
#define M3_B    15
#define M3_A    2
#define M4_B    4
#define M4_A    16

#define CONNECTED   23

#define WIFI_SSID  "codinglab5G(2)"
#define WIFI_PASSWD "codinglab"

String macAddress = "";
String ipAddress = "";

const char *websockets_server_host = "cobot.center";
const uint16_t websockets_server_port = 8286;
using namespace websockets;
WebsocketsClient client;

// set the pins to shutdown
#define SHT_LOX1 32
#define SHT_LOX2 33
#define SHT_LOX3 25
#define SHT_LOX4 13

#define LOX1_ADDRESS 0x40
#define LOX2_ADDRESS 0x41
#define LOX3_ADDRESS 0x42
#define LOX4_ADDRESS 0x43

// Sensor variables
int front_sensor,rear_sensor, left_sensor, right_sensor;
int current_angle, start_angle, end_angle;

int beat_count = 0;
int turnState = 0;

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox4 = Adafruit_VL53L0X();

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;
VL53L0X_RangingMeasurementData_t measure3;
VL53L0X_RangingMeasurementData_t measure4;


bool isTurnOver = true;
void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    //if (message.isText())
    //{
    //    const char *command = message.c_str();
    //    Serial.println(command);
  
    //}
     
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
            //stop();
            turn_left();

        }
        if (strncmp(command, "AR", 2) == 0){
            //stop();
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
  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!
  lox3.rangingTest(&measure3, false); // pass in 'true' to get debug data printout!
  lox4.rangingTest(&measure4, false); // pass in 'true' to get debug data printout!

  // print sensor one reading
  //Serial.print("1: ");
  if(measure1.RangeStatus != 4) {     // if not out of range
    front_sensor = measure1.RangeMilliMeter;    
    //Serial.print(sensor1);
    //Serial.print("mm");    
  } else {
    //Serial.print("Out of range");
  }
  
  //Serial.print(" ");

  // print sensor two reading
  //Serial.print("2: ");
  if(measure2.RangeStatus != 4) {
    rear_sensor = measure2.RangeMilliMeter;
    //Serial.print(sensor2);
    //Serial.print("mm");
  } else {
    //Serial.print("Out of range");
  }

  // print sensor three reading
  //Serial.print("3: ");
  if(measure3.RangeStatus != 4) {
    left_sensor = measure3.RangeMilliMeter;
    //Serial.print(sensor3);
    //Serial.print("mm");
  } else {
  //  Serial.print("Out of range");
  }

  // print sensor four reading
  //Serial.print("3: ");
  if(measure3.RangeStatus != 4) {
    right_sensor = measure4.RangeMilliMeter;
    //Serial.print(sensor3);
    //Serial.print("mm");
  } else {
  //  Serial.print("Out of range");
  }
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
  }
}

void go_forward(){
  Serial.println("forward");
   digitalWrite(M1_A, LOW);
    digitalWrite(M1_B, HIGH);
    digitalWrite(M2_A, LOW);
    digitalWrite(M2_B, HIGH);

    digitalWrite(M3_A, HIGH);
    digitalWrite(M3_B, LOW);
    digitalWrite(M4_A, HIGH);
    digitalWrite(M4_B, LOW);
   
}

void go_backward(){
  Serial.println("backward");
   digitalWrite(M1_A, HIGH);
    digitalWrite(M1_B, LOW);
    digitalWrite(M2_A, HIGH);
    digitalWrite(M2_B, LOW);

    digitalWrite(M3_A, LOW);
    digitalWrite(M3_B, HIGH);
    digitalWrite(M4_A, LOW);
    digitalWrite(M4_B, HIGH);
}

void turn_right(){
  Serial.println("right");
    digitalWrite(M1_A, LOW);
    digitalWrite(M1_B, HIGH);
    digitalWrite(M2_A, LOW);
    digitalWrite(M2_B, HIGH);

    digitalWrite(M3_A, LOW);
    digitalWrite(M3_B, HIGH);
    digitalWrite(M4_A, LOW);
    digitalWrite(M4_B, HIGH);
}

void turn_left(){
  Serial.println("left");
    digitalWrite(M1_A, HIGH);
    digitalWrite(M1_B, LOW);
    digitalWrite(M2_A, HIGH);
    digitalWrite(M2_B, LOW);

    digitalWrite(M3_A, HIGH);
    digitalWrite(M3_B, LOW);
    digitalWrite(M4_A, HIGH);
    digitalWrite(M4_B, LOW);
}

void stop(){
  Serial.println("stop");
    digitalWrite(M1_A, LOW);
    digitalWrite(M1_B, LOW);
    digitalWrite(M2_A, LOW);
    digitalWrite(M2_B, LOW);
    
    digitalWrite(M3_A, LOW);
    digitalWrite(M3_B, LOW);
    digitalWrite(M4_A, LOW);
    digitalWrite(M4_B, LOW);
    delay(200);
}


void setup() {
  Serial.begin(115200);
  Serial.print("ACT ground robot Start...");

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  pinMode(SHT_LOX3, OUTPUT);
  pinMode(SHT_LOX4, OUTPUT);

  pinMode(M1_A, OUTPUT);
  pinMode(M1_B, OUTPUT);
  pinMode(M2_A, OUTPUT);
  pinMode(M2_B, OUTPUT);
  pinMode(M3_A, OUTPUT);
  pinMode(M3_B, OUTPUT);
  pinMode(M4_A, OUTPUT);
  pinMode(M4_B, OUTPUT);

  pinMode(CONNECTED, OUTPUT);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  //ESP32PWM::allocateTimer(0);
	//ESP32PWM::allocateTimer(1);
	//ESP32PWM::allocateTimer(2);
	//ESP32PWM::allocateTimer(3);

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
  // c3rl3c86n88jq9lrl3gg MarsRover-1
  while (!client.connect(websockets_server_host, websockets_server_port, "/pang/ws/pub?channel=c3rl3c86n88jq9lrl3gg&track=colink&mode=bundle"))
  {
      delay(500);
      Serial.print(".");
  }
  

  Serial.println("Websocket Connected.");
  Wire.begin();

  delay(500);

  byte status = mpu.setup(0x68);
  Serial.print(F("MPU9250 status: "));
  Serial.println(status);
  while(status!=1){ 
    delay(50);
    byte status = mpu.setup(0x68);
    Serial.print(F("MPU9250 status: "));
    Serial.println(status);
    if(status == 1)
      break;
  } 
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);

  Serial.println("VL53L0X: Both in reset mode...(pins are low)");
  Serial.println("VL53L0X: Starting...");

  setID();
  delay(500);
}

/*
  status 0: first streight scanning 
  status 1: turn 90 degree  
  status 3: second streight scanning 
  status 4: draw map - find where I am where is object 
  status 5: self driving to object
  status -1: it is not SLAM operation 
*/
static int slam_status = -1 
void slam_move(){
  if(status == 0){
    if(front_snesor < 200){
      stop();
      slam_status = 1;
      delay(100);
    }else{
      go_forward();
      delay(100)
    }
  }else if(status == 1){
    if( left_sensor > right_sensor){
      turn_left();
    }else{
      turn_right();
    }
  }else if(status == 2){

  }else if(status == 3){

  }else if(status == 4){

  }else{

  }
  status++;
  if(status > 4)
    status = 0;
}


void loop() {
  if(Serial.available() > 0){
    char c = Serial.read();
    Serial.println(c);
      if(c == 'f'){
        //stop();
        go_forward();
    }else if(c == 'b'){
        //stop();
        go_backward();
    }else if(c == 'l'){
        //stop();
        turn_left();
        /* for controlled turn 
        end_angle = current_angle + 90;
        isTurnOver = false;
        turnState = 1;
        */
    }else if(c == 'r'){
        //stop();
        turn_right(); 
        /* for controlled turn
        end_angle = current_angle + (-90);
        isTurnOver = false;
        turnState = 2;
        */
    }else if(c == 'p'){
        Serial.println("stop");
        stop();
        isTurnOver = true;
    }else if(c == 'h'){
      slam_status = 0;
    }
  }
  if (mpu.update()) {
    static uint32_t prev_ms = millis();
    if (millis() > prev_ms + 25) {
      current_angle = mpu.getYaw(); 
      read_dual_sensors();
      prev_ms = millis();
    }
  }

  client.poll();
  beat_count++;
  if(beat_count > 10){
    String data = 'a'+String(current_angle)+'b'+String(front_sensor)+'c'+String(rear_sensor)+'d'+String(left_sensor)+'e'+String(right_sensor)+'f';
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

