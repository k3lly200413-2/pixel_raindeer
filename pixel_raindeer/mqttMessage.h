#include <Arduino.h>

void mqttSetup();

void callback(char *topic, byte *payload, unsigned int length);

void publish(char* topic, const uint8_t* message);

void sub(char* topic);

void onMqttMessage(char* topic, byte* payload, unsigned int length);

void clientLoop();
