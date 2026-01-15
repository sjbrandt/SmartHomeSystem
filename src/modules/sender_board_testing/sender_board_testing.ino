#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t broadcastAddress[] = { 0xC8, 0x2B, 0x96, 0x09, 0x0E, 0x47 };

typedef struct message_struct {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} struct_message;

message_struct received_data;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;  // send readings timer
float tries = 0;
float successes = 0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.print("Delivery success.");
    successes++;
  } else {
    Serial.print("Delivery fail.");
  }
  tries++;
  Serial.print(" Success rate: ");
  Serial.print((successes/tries)*100);
  Serial.println("%");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    strcpy(received_data.a, "THIS IS A CHAR");
    received_data.b = random(1, 20);
    received_data.c = 1.2;
    received_data.d = "Hello";
    received_data.e = false;

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&received_data, sizeof(received_data));

    lastTime = millis();
  }
}
