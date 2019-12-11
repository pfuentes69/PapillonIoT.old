#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

#define DHTPIN D4     // what pin we're connected to

DHTesp dht;

IPAddress server(192,168,2,101);

const char* ssid     = "Papillon";
const char* password = "70445312";

int status = WL_IDLE_STATUS;   // the Wifi radio's status

// sleep for this many seconds
const int sleepSeconds = 600;

// Initialize the Ethernet client object
WiFiClient WIFIclient;

PubSubClient client(WIFIclient);

void setup() {
  // SETUP START

  // initialize digital pin LED_BUILTIN as an output.
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)

  // initialize serial for debugging
  Serial.begin(115200);

    Serial.println();
  Serial.println("******* SETUP START *******");

  // Connect D0 to RST to wake up
 pinMode(D0, WAKEUP_PULLUP);

  // initialize WiFi
  WiFi.mode(WIFI_STA);

  bool connectingWIFI = true;
  int tries = 0;

  while (connectingWIFI) {
    WiFi.begin(ssid, password);
    tries = 0;    
    Serial.println("Trying Main WiFi");
    while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
      delay(500);
      Serial.print(".");
      tries++;
    }
    Serial.println();

/*
    if (tries == 10) {
      WiFi.begin(ssidAlt, passwordAlt);
      tries = 0;    
      Serial.println("Trying Alternate WiFi");
      while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
        delay(500);
        Serial.print(".");
        tries++;
      }
      Serial.println();
    }
*/  
    if (tries == 10) {
      Serial.println("Too many trials, we'll sleep and try later");
      ESP.deepSleep(sleepSeconds * 1000000);
    } else {
      connectingWIFI = false;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //connect to MQTT server
  client.setServer(server, 1883);

  dht.setup(DHTPIN, DHTesp::DHT22); // data pin 4

  delay(1000); // Stabilize Sensors

  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  String payload = "{\"Temperature\":";
  payload += temperature;
  payload += ",\"Humidity\":";
  payload += humidity;
  payload += "}";

  Serial.println(payload);
  
  client.publish("PapillonIoT/DHT22Sensor1/data", (char*) payload.c_str());
  delay(1000);  
  Serial.println("Deep Sleep");
  // Let's go to sleep
  ESP.deepSleep(sleepSeconds * 1000000);

}

//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void loop() {
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (client.connect("DHT22Sensor1")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      client.publish("Papillon/dev02/status","hello world");
      // ... and resubscribe
//      client.subscribe("presence");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
