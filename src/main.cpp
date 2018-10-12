/* MQTT-Arduino doorbell
   David Pye (C) 2016 GNU GPL v3
   Based on https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
   */


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

//Pin definition for other peripherals
#define RELAY_PIN 5
#define BUTTON_PIN 4 //I2C_SDA_PIN
#define LED_PIN 15

const char *ssid="";
const char *password = "";
const char *mqtt_server = "192.168.1.50";

const char *name = "mqtt_doorbell";

const char *buttonPressedTopic = "status/doorbell/pressed";
const char *bellRangTopic = "status/doorbell/rang";

const char *bellMuteTopic = "cmnd/doorbell/mute";
const char *bellRingTopic = "cmnd/doorbell/ring";

//Default to non-muted doorbell status.
bool muted = false;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel led = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);


void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  for (int retry_count = 0; retry_count <20 && WiFi.status() != WL_CONNECTED; ++retry_count) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status () == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else Serial.println("Failed to connect to wifi - abandoned");

}

void reconnect() {
  // Loop until we're reconnected
  for (int retry_count = 0; retry_count < 20 && !client.connected(); ++retry_count) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect(name)) {
      Serial.println("connected");
      client.subscribe(bellMuteTopic);
      client.subscribe(bellRingTopic);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void ringBell() {
  //Ring the bell!
  Serial.println("Ringing the bell!");
  //Ding
  digitalWrite(RELAY_PIN, HIGH);
  delay(400);
  //Dong
  digitalWrite(RELAY_PIN, LOW);
  client.publish(bellRangTopic, "1");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if (!strcmp(topic, bellMuteTopic)) {
    if ((char)payload[0] == '1') {
      muted = true;
      //Red
      led.setPixelColor(0,255,0,0);
      led.show();
    }
    else if ((char)payload[0] == '0') {
      muted = false;
      //Green
      led.setPixelColor(0,0,255,0);
      led.show();
    }
  }
  else if (!strcmp(topic, bellRingTopic)) {
    ringBell();
  }
}


void setup() {
  led.begin();
  led.setBrightness(10);
  led.setPixelColor(0,255,255,0);
  led.show();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT);

  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  led.setPixelColor(0,0,255,0);
  led.show();
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Doorbell pressed");
    //Ooh, someone has rung the doorbell!
    if (!muted)
      ringBell();
    else
      Serial.println("Muted - not ringing");

    client.publish(buttonPressedTopic, "1");
    delay(1000);
  }

  delay(50);
}
