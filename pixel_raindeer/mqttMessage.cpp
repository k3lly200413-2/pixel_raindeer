#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "mqttMessage.h"
#include "displayQueue.h"
#include "Config.h"


// WiFi
const char *ssid = WIFI_SSID; // Enter your WiFi name
const char *password = WIFI_PASSWORD;  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = MQTT_HOST;
const char *topic = TOPIC;
const char *mqtt_username = MQTT_USER;
const char *mqtt_password = MQTT_PASS;
const int mqtt_port = 8883;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void mqttSetup()
{
    // Connecting to a Wi-Fi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println("Connected to the Wi-Fi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    
    while (!client.connected()) {
        
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        espClient.setInsecure();
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } 
        else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // Publish and subscribe
    client.publish(topic, "Hi, I'm ESP32 ^-^");
    client.subscribe(topic);
}

void sub(char* topic)
{
    client.subscribe(topic);
}

void publish(char* topic, const uint8_t* message)
{
    client.publish(topic, message, (sizeof(message)/sizeof(uint8_t)), 2);
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println("We got the message");
    char buf[24];
    unsigned int copyLen = (length < sizeof(buf) - 1) ? length : sizeof(buf) - 1; // avoid overflow if payload is unexpectedly long
    memcpy(buf, payload, copyLen);
    buf[copyLen] = '\0';  // null-terminate — payload isn't guaranteed to be

    DisplayCommand cmd;
    if (sscanf(buf, "%d,%d", &cmd.packIndex, &cmd.animIndex) == 2) {
        xQueueSend(displayQueue, &cmd, 0);
    }
}

void clientLoop()
{
    client.loop();
}
