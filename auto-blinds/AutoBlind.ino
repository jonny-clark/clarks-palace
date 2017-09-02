#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Server settings
const char* ssid = "ken13";
const char* password = "bu5yb33s";
const char* mqtt_server = "192.168.1.135";
ESP8266WebServer server(80);
WiFiClient myWiFi;
PubSubClient client(myWiFi);


// Pin assignment
const int motorPin1 = 2;
const int motorPin2 = 4;
const int irSwitchPin = A0;

// Variables to keep track of the blind rotations
const int minValue = 0;
const int maxValue = 8;
int currentValue;

int sensorValue = 0;

// The possible states that the blind can be in
enum blindStates {
  GOINGUP,
  GOINGDOWN,
  STOPPED
};
enum blindStates blindState;

// Start moving the blind up
void moveBlindUp() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  blindState = GOINGUP;
  Serial.println("Moving up!");
}

void checkGoingUpState() {
  checkCurrentValue();
  sensorValue = analogRead(irSwitchPin);
  if(sensorValue > 600) {
    changeCurrentValue(true);
    if(currentValue == maxValue) {
      stopBlind();
      client.publish("state", "Open");
    } else {
      delay(1000);
    }
  }
}

// Start moving the blind down
void moveBlindDown() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  blindState = GOINGDOWN;
  Serial.println("Moving down!");
}

void checkGoingDownState() {
  checkCurrentValue();
  sensorValue = analogRead(irSwitchPin);
  if(sensorValue > 600) {
    changeCurrentValue(false);
    if(currentValue == minValue) {
      stopBlind();
      client.publish("state", "Closed");
    } else {
      delay(1000);
    }
  }
}

// Stop the blind no matter where it is
void stopBlind() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  blindState = STOPPED;
}

void checkCurrentValue() {
  if (currentValue < minValue) {
    stopBlind();
    currentValue = minValue;
  }
  if (currentValue > maxValue) {
    stopBlind();
    currentValue = maxValue;
  }
}

void changeCurrentValue(bool up) {
  if(up) {
    currentValue += 1;
  } else {
    currentValue -= 1;
  }
  Serial.print("Current value is now ");
  Serial.println(currentValue);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print(String((char*)payload));
  Serial.println();

  if ((String)topic == "blind" && String((char*)payload).indexOf("OPEN") != -1) {
    if(currentValue < maxValue) {
      Serial.println("Moving Up");
      moveBlindUp();
    } else {
      client.publish("state", "Open");
    }
  }
  if ((String)topic == "blind" && String((char*)payload).indexOf("CLOSE") != -1) {
    if(currentValue > minValue) {
      Serial.println("Moving Down");
      moveBlindDown();
    } else {
      client.publish("state", "Closed");
    }
  }
  if ((String)topic == "blind" && String((char*)payload).indexOf("STOP") != -1) {
    Serial.println("Stopping");
    stopBlind();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "BlindClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ping", "I'm Alive");
      // ... and resubscribe
      client.subscribe("blind");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);

  blindState = STOPPED;
  currentValue = minValue;

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// the loop function runs over and over again forever
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  switch(blindState) {
    case STOPPED:
      break;
    case GOINGUP:
      checkGoingUpState();
      break;
    case GOINGDOWN:
      checkGoingDownState();
      break;
  }
  
  client.loop();
}
