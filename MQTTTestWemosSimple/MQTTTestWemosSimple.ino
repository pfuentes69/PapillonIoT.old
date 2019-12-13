#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//IPAddress server(192,168,2,10); // Casa

IPAddress server(35,178,36,205); // Solace

const char* ssid     = "Naviter"; //Papillon";
const char* password = "N4v1t3rWIFI2015"; //70445312";

int status = WL_IDLE_STATUS;   // the Wifi radio's status

// Initialize the Ethernet client object
WiFiClient WIFIclient;

PubSubClient client(WIFIclient);

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //connect to MQTT server
  client.setServer(server, 1883);
  client.setCallback(callback);

  client.connect("dev01", "dev01", "dev01");

  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");

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
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  } else {

    float humidity = 89;
    float temperature = 25;
  
    String payload = "{\"temperature\":";
    payload += temperature;
    payload += ",\"humidity\":";
    payload += humidity;
    payload += "}";

    Serial.println(payload);
    
    client.publish("test/dev01", (char*) payload.c_str());
  }
  client.loop();
  delay(3000);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (client.connect("dev02", "dev02", "dev02")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("Papillon/dev02/status","hello world");
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

