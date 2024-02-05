#include <ESP8266WiFi.h>
#include "PubSubClient.h"

const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
const char *mqtt_server = "YOUR_MQTT_SERVER";
const char *mqtt_name = "YOUR_MQTT_SERVER_NAME";
const char *mqtt_password = "YOUR_MQTT_SERVER_PASSWORD";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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
  String message;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  if (message == "start") {
    // Start Button
    digitalWrite(2, HIGH);
    snprintf(msg, MSG_BUFFER_SIZE, "%ld", 1);
    delay(500);
    digitalWrite(2, LOW);
    snprintf(msg, MSG_BUFFER_SIZE, "%ld", 0);
  } else if (message == "return_to_base") {
    // Return Button
    digitalWrite(4, HIGH);
    snprintf(msg, MSG_BUFFER_SIZE, "%ld", 1);
    delay(500);
    digitalWrite(4, LOW);
    snprintf(msg, MSG_BUFFER_SIZE, "%ld", 0);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_name, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("vacuum/power/outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("vacuum/power/inTopic");
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
  pinMode(2, OUTPUT); // GPIO02 (D4)
  pinMode(4, OUTPUT); // GPIO04 (D2)
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    client.publish("vacuum/power/outTopic", msg);
  }
}