/**
 * A program for the WeMos D1 to set the LED in response to
 * requests from HomeBridge.
 * 
 * Hardware assumptions:
 * Onboard LED (Pin 4)
 */
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
 
const char* ssid = "Papillon";
const char* password = "70445312";

const int relayPin = D1;
const int ledPin = D4;
const int buttonPin = D5;
const int LDRPin = A0;
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
const int thresholdLDR = 700; // Light level to turn on Lamp
const int delayLDR = 200; //ms

int status = WL_IDLE_STATUS;   // the Wifi radio's status
bool estadoLampara = false;
bool viejoEstadoLampara = false;
bool forzarEstado = false;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled

ESP8266WebServer server(80);
 
void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}
 
void handleNotFound() {
  String message = "File Not Found\n";
 
  server.send(404, "text/plain", message);
}
 
void setup(void){
  Serial.begin(115200);
  Serial.println("<<<<< SETUP START >>>>>");
//  pinMode(LED_BUILTIN, OUTPUT);

   // Set Relay
  pinMode(relayPin, OUTPUT);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ledPin, OUTPUT);

  // Set Button
  pinMode(buttonPin, INPUT_PULLUP);

  bool connectingWIFI = true;
  int tries = 0;

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  Serial.println("Trying Main WiFi");
  while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();
  if (tries >= 10) {
    Serial.println("Too many trials, no WiFi connection was possible");
  } else {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Network OK");
   
    if (MDNS.begin("papilloniot-lampara")) {
      Serial.println("MDNS responder started");
    }
   
    server.on("/", [](){
      server.send(200, "text/plain", "PapillonIoT Lamp");
    });
   
    server.on("/on", [](){
      server.send(200, "text/plain", "Light is on");
      Serial.println("Light on");
      digitalWrite(LED_BUILTIN, 0);
        estadoLampara = true;
    });
   
    server.on("/off", [](){
      server.send(200, "text/plain", "Light is off");
      Serial.println("Light off");
      digitalWrite(LED_BUILTIN, 1);
      estadoLampara = false;
    });
   
    server.on("/status", [](){
      if (estadoLampara) 
        server.send(200, "text/plain", "1");
      else
        server.send(200, "text/plain", "0");
        
      Serial.println("Get Status");
    });

    server.on("/ldr", [](){
      int sensorValue = analogRead(LDRPin);
      String sValue = String(1024 - sensorValue);
      String payload = "{ \"light\": " + sValue +"}";
      server.send(200, "text/plain", payload);
      Serial.println("Get LDR");
    });
   
    server.onNotFound(handleNotFound);
   
    server.begin();
    Serial.println("HTTP server started");
  }

  // Estado inicial
  controlLampara(estadoLampara);
  Serial.println("<<<<< SETUP END >>>>>");
}
 
void loop(void) {
  viejoEstadoLampara = estadoLampara;
  
  server.handleClient();
    // Update Lampara Status and sensors
  updateLampara();

  // Check new status
  if (estadoLampara != viejoEstadoLampara) {
     // set the Lampara:
    controlLampara(estadoLampara);
  }
}

/////////////////////////////////////
//
// CUSTOM HW FUNCITONS
//
/////////////////////////////////////

void controlLED (bool OnOff)
{
  if (OnOff) {
    Serial.println("Encender LED");
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (LOW is the voltage level)
  }
  else {
    Serial.println("Apagar LED");
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off (HIGH is the voltage level)    
  }
}

void controlRelay (bool OnOff)
{
  if (OnOff) {
    Serial.println("Encender Relay");
    digitalWrite(relayPin, HIGH);   // turn the RELAY on (HIGH is the voltage level)    
  } 
  else {
    Serial.println("Apagar Relay");
    digitalWrite(relayPin, LOW);   // turn the RELAY off (LOW is the voltage level)
  }
}

void controlLampara (bool OnOff)
{
  if (OnOff) {
    Serial.println("Encender Lampara");
    controlLED(false);
    controlRelay(true);
  }
  else {
    Serial.println("Apagar Lampara");
    controlLED(true);
    controlRelay(false);
  }
}

void controlButton() 
{
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == LOW) {
        estadoLampara = !estadoLampara;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void controlLDR()
{
  if (forzarEstado) {
    // We terminate here
    return;
  }
  
  int sensorValue = analogRead(LDRPin);

  if (estadoLampara && sensorValue < thresholdLDR) {
    // Hay luz, apagamos
    estadoLampara = false;
  } else if (!estadoLampara && sensorValue >= thresholdLDR) {
    // No hay luz, encendemos
    estadoLampara = true;
  }

  delay(delayLDR);
}

void updateLampara()
{
  // Read analog
//  controlLDR();
  
  // Read button
  controlButton();

}

