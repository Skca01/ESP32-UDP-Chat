#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <AsyncUDP.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Name";
const char* password = "Password";

const uint16_t UDP_PORT = 3333;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncUDP udp;

struct ChatClient {
    IPAddress ipAddress;
    uint32_t lastSeen;
    String username;
    
    ChatClient(IPAddress addr, uint32_t time, String name) 
        : ipAddress(addr), lastSeen(time), username(name) {}
};

std::vector<ChatClient> chatClients;

void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setupMDNS() {
    if (MDNS.begin("esp32chat")) {
        Serial.println("mDNS started: http://esp32chat.local");
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 80);
        MDNS.addService("udp", "udp", UDP_PORT);
    }
}

void broadcastUDPMessage(const char* message, const char* sender, IPAddress excludeIP) {
    StaticJsonDocument<200> doc;
    doc["message"] = message;
    doc["sender"] = sender;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    for (const auto& client : chatClients) {
        if (client.ipAddress != excludeIP) {
            udp.writeTo((uint8_t*)jsonString.c_str(), jsonString.length(), client.ipAddress, UDP_PORT);
        }
    }
}

void handleWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        StaticJsonDocument<200> doc;
        data[len] = 0;
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (!error) {
            String message = doc["message"];
            String sender = doc["sender"];
            
            StaticJsonDocument<200> responseDoc;
            responseDoc["message"] = message;
            responseDoc["sender"] = sender;
            
            String responseString;
            serializeJson(responseDoc, responseString);
            
            for (auto& wsClient : ws.getClients()) {
                if (wsClient->id() != client->id()) {
                    wsClient->text(responseString);
                }
            }
            
            broadcastUDPMessage(message.c_str(), sender.c_str(), client->remoteIP());
        }
    }
}

void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
                     void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(client, arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void setupUDP() {
    if (udp.listen(UDP_PORT)) {
        Serial.print("UDP Listening on port: ");
        Serial.println(UDP_PORT);
        
        udp.onPacket([](AsyncUDPPacket packet) {
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, packet.data());
            
            if (!error) {
                String message = doc["message"];
                String sender = doc["sender"];
                
                StaticJsonDocument<200> responseDoc;
                responseDoc["message"] = message;
                responseDoc["sender"] = sender;
                
                String responseString;
                serializeJson(responseDoc, responseString);
                
                ws.textAll(responseString);
                
                bool clientExists = false;
                for (auto& client : chatClients) {
                    if (client.ipAddress == packet.remoteIP()) {
                        client.lastSeen = millis();
                        client.username = sender;
                        clientExists = true;
                        break;
                    }
                }
                if (!clientExists) {
                    chatClients.emplace_back(packet.remoteIP(), millis(), sender);
                }
            }
        });
    }
}

void cleanupStaleClients() {
    const uint32_t TIMEOUT = 60000;
    auto now = millis();
    
    chatClients.erase(
        std::remove_if(
            chatClients.begin(),
            chatClients.end(),
            [now, TIMEOUT](const ChatClient& client) {
                return (now - client.lastSeen) > TIMEOUT;
            }
        ),
        chatClients.end()
    );
}

void setup() {
    Serial.begin(115200);
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    
    setupWiFi();
    setupMDNS();
    setupUDP();
    
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });
    
    server.begin();
    Serial.println("Server started");
}

void loop() {
    ws.cleanupClients();
    cleanupStaleClients();
    delay(100); 
}
