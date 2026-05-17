#include <WiFi.h>
#include <PubSubClient.h>

// --- 1. NETWORK & BROKER SETTINGS ---
const char* ssid = "Nadeem";        // CHANGE THIS
const char* password = "Pak123456"; // CHANGE THIS
const char* mqtt_server = "192.168.18.74";     // CHANGE THIS to your PC's IP

const char* command_topic = "robot/move";
const char* status_topic = "robot/status";

WiFiClient espClient;
PubSubClient client(espClient);


// LEFT Motor (Connected to OUT1/OUT2)
#define ENA 14
#define IN1 27
#define IN2 26

// RIGHT Motor (Connected to OUT3/OUT4)
#define ENB 32
#define IN3 25
#define IN4 33

// PHASE 2: Fire Extinguishing Hardware (Ready for later)
#define PUMP_RELAY 12
#define FLAME_SENSOR 13

// Speed Setting (0 to 255)
const int motorSpeed = 200; 

void setup() {
  Serial.begin(115200);
  
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(PUMP_RELAY, OUTPUT); pinMode(FLAME_SENSOR, INPUT); 
  
  stopMotors(); // Ensure motors are off at boot
  digitalWrite(PUMP_RELAY, LOW); 

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Phase 2 Preview: Fire Detection
  if (digitalRead(FLAME_SENSOR) == LOW) {
    // Fire detected logic will go here
  }
}

// --- MQTT & WIFI SETUP ---
void setup_wifi() {
  delay(10);
  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected! IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-FireRobot-";
    clientId += String(random(0, 0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to Mosquitto!");
      client.subscribe(command_topic); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// --- INCOMING MESSAGE HANDLER ---
void callback(char* topic, byte* payload, unsigned int length) {
  // Grab the very first character of the incoming message
  char command = (char)payload[0];
  
  Serial.print("Command Received: ");
  Serial.println(command);

  // Match the character to your Bluetooth commands
  if (command == 'U') {        // 'U' for Up/Forward
    moveForward();
  } 
  else if (command == 'D') {   // 'D' for Down/Backward
    moveBackward();
  } 
  else if (command == 'L') {   // Left
    turnLeft();
  } 
  else if (command == 'R') {   // Right
    turnRight();
  } 
  else if (command == 'S') {   // Stop
    stopMotors();
  }
}

// --- STANDARD MOTOR MOVEMENT FUNCTIONS ---

void moveForward() {
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
  
  // Both motors spin forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
  
  // Both motors spin backward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnRight() {
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
  
  // Left spins forward, Right spins backward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
  
  // Left spins backward, Right spins forward
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopMotors() {
  // All pins low = coast to a stop
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}