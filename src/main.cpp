#include <Arduino.h>
#include <Bounce2.h>
#include <WiFiNINA.h>

const int WaterFlow = 5;
const int WaterRelay = 3;

#define ON LOW
#define OFF HIGH

Bounce waterFlow;
WiFiServer telnet(23);

bool washMachine = false;
bool manual = false;
bool wifi = false;
bool waterRelay = false;
bool waterFlowState = false;
void handleRequest();

void setWaterRelay(bool _wr) {
  digitalWrite(WaterRelay, _wr ? ON : OFF);
  waterRelay = _wr;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(WaterRelay, OUTPUT);
  setWaterRelay(false);
  waterFlow.attach(WaterFlow, INPUT);
  waterFlow.interval(100);

  int ap_state = WiFi.beginAP("HOMECTRL");
  if (ap_state == WL_CONNECT_FAILED) {
    wifi = false;
    pinMode(LED_BUILTIN, HIGH);
  }
  else if (ap_state == WL_AP_LISTENING) {
    wifi = true;
  }
  if (wifi) {
    telnet.begin();
  }
  washMachine = false;
}

void loop() {
  waterFlow.update();
  waterFlowState = waterFlow.read();
  if (!washMachine)
    setWaterRelay(waterFlowState);
  else
    setWaterRelay(true);
  handleRequest();
}

void handleRequest() {
  WiFiClient client = telnet.available();
  
  int inc;
  String instr;

  if (!client)
    return;

  while (client.connected()) {
    delayMicroseconds(10);
    if (client.available()) {
      inc = client.read();
      if (inc != '\r' && inc != '\n') {
        instr += (char)inc;
      } else {
        if (inc == '\r')
          continue;

        Serial.println(instr);
        switch (instr[0]) {
          case 'W':
            if (instr[1] == '0') {
              washMachine = false;
            } else if (instr[1] == '1') {
              washMachine = true;
              setWaterRelay(true);
            } else {
              client.println("WE");
              break;
            }
            client.println("OK");
            break;
          case 'I':
            client.println(washMachine ? "W1" : "W0");
            client.println(waterRelay  ? "R1" : "R0");
            client.println(waterFlowState ? "w1" : "w0");
            break;
          default:
            client.println("!E");
        }

        client.stop();
      }
    }
  }
}
