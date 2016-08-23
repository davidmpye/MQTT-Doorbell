/* MQTT-Arduino doorbell
   David Pye (C) 2016 GNU GPL v3
   Based on https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
   */


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Pin definition for other peripherals
#define RELAY_PIN 5
#define BUTTON_PIN 4 //I2C_SDA_PIN

const char *ssid="";
const char *password = "";
const char *mqtt_server = "192.168.1.50";

const char *name = "mqtt_doorbell";

const char *bellPressedTopic = "/status/doorbell/pressed";
const char *bellControlTopic = "/cmnd/doorbell/shouldring";

//Default to non-muted doorbell status.
boolean shouldRing = true;

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect(name)) {
      Serial.println("connected");
      client.subscribe(bellControlTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if ((char)payload[0] == '1') {
    shouldRing = true;
  }
  else if ((char)payload[0] == '0') {
    shouldRing = false;
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT);

  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Doorbell pressed");
    //Ooh, someone has rung the doorbell!
    if (shouldRing) {
      Serial.println("Ringing the bell!");
      //Ding
      digitalWrite(RELAY_PIN, HIGH);
      delay(500);
      //Dong
      digitalWrite(RELAY_PIN, LOW);
    }
    else {
      Serial.println("Muted - not ringing");
    }
    client.publish(bellPressedTopic, "1");
    delay(1000);
  }

  delay(50);
}
