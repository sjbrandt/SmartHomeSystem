#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ThingSpeak.h>

const char* ssid = "TheBlackLodge";
const char* pass = "theowlsarenotwhattheyseem";
const uint8_t ledPin = D2;
WiFiClient client;

unsigned long channelID = 3226650;             //your TS channal
const char* APIKeyWrite = "4Q7TRKUR9M8JU1AS";  //your TS API
const char* APIKeyRead = "5ZB9IIYG16N9N9ED";
const char* server = "api.thingspeak.com";
const int postDelay = 20 * 1000;  //post data every 20 seconds

typedef struct message_struct {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} message_struct;

message_struct receivedData;

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(receivedData.a);
  Serial.print("Int: ");
  Serial.println(receivedData.b);
  Serial.print("Float: ");
  Serial.println(receivedData.c);
  Serial.print("String: ");
  Serial.println(receivedData.d);
  Serial.print("Bool: ");
  Serial.println(receivedData.e);
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(ssid, pass);

  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  Serial.print("MAC address is: ");
  Serial.println(WiFi.macAddress());
  
  esp_now_init();

  ThingSpeak.begin(client);

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // put your main code here, to run repeatedly:

}
