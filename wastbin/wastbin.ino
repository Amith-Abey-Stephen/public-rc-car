#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
#define MAX_NETWORKS 4
const char* ssids[MAX_NETWORKS] = { "KCMT-LAB","Chacko Mash","KCMT-ENG-FLR-A",  "master"  };
const char* passwords[MAX_NETWORKS] = { "1ab2ac3ad4ae5af","marannupoi","1ab2ac3ad4ae5af",  "123456789"  };

// API endpoint
const char* apiEndpoint = "https://public-rc-car.onrender.com/status";

// Define the motor control pins for L298N
#define MOTOR_LEFT_ENA 26
#define MOTOR_LEFT_IN1 25
#define MOTOR_LEFT_IN2 33
#define MOTOR_RIGHT_ENB 27
#define MOTOR_RIGHT_IN1 14
#define MOTOR_RIGHT_IN2 12

// Define LED pins
#define WIFI_LED_PIN 13
#define ERROR_LED_PIN 2
#define FORWARD_LED_PIN 15
#define REVERSE_LED_PIN 0
#define LEFT_LED_PIN 4
#define RIGHT_LED_PIN 16
#define STOP_LED_PIN 17

// Define speed constants
#define MAX_SPEED 255
#define HIGH_SPEED 200  // ~78% of max speed
#define MEDIUM_SPEED 150  // ~10% of max speed
#define LOW_SPEED 150  // ~10% of max speed

unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 1000; // Fetch every 1 second

void printLoader(const char* message) {
  static const char loader[] = {'|', '/', '-', '\\'};
  static int loaderIndex = 0;
  Serial.printf("\r%s %c", message, loader[loaderIndex]);
  loaderIndex = (loaderIndex + 1) % 4;
}

bool connectToWiFi() {
  digitalWrite(WIFI_LED_PIN, LOW);
  
  for (int i = 0; i < MAX_NETWORKS; i++) {
    if (strlen(ssids[i]) == 0) break;
    Serial.printf("\nAttempting to connect to %s\n", ssids[i]);
    WiFi.begin(ssids[i], passwords[i]);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      printLoader("Connecting to WiFi");
      delay(500);
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi successfully");
      Serial.printf("Connected to: %s\n", ssids[i]);
      Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
      digitalWrite(WIFI_LED_PIN, HIGH);
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8,8,8,8));
      return true;
    } else {
      Serial.printf("\nFailed to connect to %s\n", ssids[i]);
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 RC Car Control - Initializing...");
  
  // Set all pins as outputs
  pinMode(MOTOR_LEFT_ENA, OUTPUT);
  pinMode(MOTOR_LEFT_IN1, OUTPUT);
  pinMode(MOTOR_LEFT_IN2, OUTPUT);
  pinMode(MOTOR_RIGHT_ENB, OUTPUT);
  pinMode(MOTOR_RIGHT_IN1, OUTPUT);
  pinMode(MOTOR_RIGHT_IN2, OUTPUT);
  pinMode(WIFI_LED_PIN, OUTPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(FORWARD_LED_PIN, OUTPUT);
  pinMode(REVERSE_LED_PIN, OUTPUT);
  pinMode(LEFT_LED_PIN, OUTPUT);
  pinMode(RIGHT_LED_PIN, OUTPUT);
  pinMode(STOP_LED_PIN, OUTPUT);
  
  // Initialize motor speed to 0
  analogWrite(MOTOR_LEFT_ENA, 0);
  analogWrite(MOTOR_RIGHT_ENB, 0);
  
  // Initialize all LEDs to OFF
  digitalWrite(WIFI_LED_PIN, LOW);
  digitalWrite(ERROR_LED_PIN, LOW);
  digitalWrite(FORWARD_LED_PIN, LOW);
  digitalWrite(REVERSE_LED_PIN, LOW);
  digitalWrite(LEFT_LED_PIN, LOW);
  digitalWrite(RIGHT_LED_PIN, LOW);
  digitalWrite(STOP_LED_PIN, LOW);
  
  Serial.println("Motor control pins and LEDs initialized");
  
  // Connect to Wi-Fi
  if (!connectToWiFi()) {
    Serial.println("Failed to connect to any WiFi network. Restarting...");
    ESP.restart();
  }

  Serial.println("Setup complete. Entering main loop...");
}

void turnOffAllLEDs() {
  digitalWrite(ERROR_LED_PIN, LOW);
  digitalWrite(FORWARD_LED_PIN, LOW);
  digitalWrite(REVERSE_LED_PIN, LOW);
  digitalWrite(LEFT_LED_PIN, LOW);
  digitalWrite(RIGHT_LED_PIN, LOW);
  digitalWrite(STOP_LED_PIN, LOW);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    digitalWrite(WIFI_LED_PIN, LOW);
    if (!connectToWiFi()) {
      Serial.println("Failed to reconnect to any WiFi network. Restarting...");
      ESP.restart();
    }
  }

  if (millis() - lastFetchTime > fetchInterval) {
    lastFetchTime = millis();
    
    HTTPClient http;
    http.begin(apiEndpoint);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("API Response: " + response);
      
      turnOffAllLEDs();
      
      if (response == "forward") {
        moveForward();
        digitalWrite(FORWARD_LED_PIN, HIGH);
      } else if (response == "reverse") {
        moveBackward();
        digitalWrite(REVERSE_LED_PIN, HIGH);
      } else if (response == "left") {
        turnLeft();
        digitalWrite(LEFT_LED_PIN, HIGH);
      } else if (response == "right") {
        turnRight();
        digitalWrite(RIGHT_LED_PIN, HIGH);
      } else if (response == "stop") {
        stop();
        digitalWrite(STOP_LED_PIN, HIGH);
      } else {
        Serial.println("Unknown command received");
        for (int i = 0; i < 3; i++) {
          digitalWrite(ERROR_LED_PIN, HIGH);
          delay(100);
          digitalWrite(ERROR_LED_PIN, LOW);
          delay(100);
        }
      }
    } else {
      Serial.print("Error on HTTP request. Error code: ");
      Serial.println(httpResponseCode);
      digitalWrite(ERROR_LED_PIN, HIGH);
    }
    
    http.end();
  }
}

void moveForward() {
  analogWrite(MOTOR_LEFT_ENA, MEDIUM_SPEED);
  analogWrite(MOTOR_RIGHT_ENB, MEDIUM_SPEED);
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  digitalWrite(MOTOR_RIGHT_IN1, HIGH);
  digitalWrite(MOTOR_RIGHT_IN2, LOW);
  Serial.println("Moving Forward");
}

void moveBackward() {
  analogWrite(MOTOR_LEFT_ENA, MEDIUM_SPEED);
  analogWrite(MOTOR_RIGHT_ENB, MEDIUM_SPEED);
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  digitalWrite(MOTOR_RIGHT_IN1, LOW);
  digitalWrite(MOTOR_RIGHT_IN2, HIGH);
  Serial.println("Moving Backward");
}

void turnLeft() {
  analogWrite(MOTOR_LEFT_ENA, LOW_SPEED);
  analogWrite(MOTOR_RIGHT_ENB, LOW_SPEED);
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  digitalWrite(MOTOR_RIGHT_IN1, HIGH);
  digitalWrite(MOTOR_RIGHT_IN2, LOW);
  Serial.println("Turning Left");
}

void turnRight() {
  analogWrite(MOTOR_LEFT_ENA, LOW_SPEED);
  analogWrite(MOTOR_RIGHT_ENB, LOW_SPEED);
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  digitalWrite(MOTOR_RIGHT_IN1, LOW);
  digitalWrite(MOTOR_RIGHT_IN2, HIGH);
  Serial.println("Turning Right");
}

void stop() {
  analogWrite(MOTOR_LEFT_ENA, 0);
  analogWrite(MOTOR_RIGHT_ENB, 0);
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  digitalWrite(MOTOR_RIGHT_IN1, LOW);
  digitalWrite(MOTOR_RIGHT_IN2, LOW);
  Serial.println("Stopped");
}