#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SCD30.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>


// Credenciais da rede Wifi
const char* ssid = "xxxxxxxxxx";
const char* password = "xxxxxxxx";

// Credenciais do broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "devices/NAPIoT-P2Vigo-Rec";
const char* mqttUser = "";
const char* mqttPassword = "";

// Variables para gardar os valores de temperatura e CO2
char CO2[10]; 
char Temp[10]; 

const int servoVentanaPin = 8;
const int servoVentiladorPin = 6;

WiFiClient espClient;
PubSubClient client(espClient);

//Librerias dos nosos compoñentes
Adafruit_SCD30 scd30; 
Servo servoVentana;
Servo servoVentilador;


// Función para conectar á WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectada!");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

// Función de callback que procesa as mensaxes MQTT recibidas
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaxe recibida[");
  Serial.print(topic);
  Serial.print("] ");
  // Imprimese o payload da mensaxe
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Comprobamos a mensaxe recibida
  Serial.println(message);
  if (message == "OPEN") {
      servoVentana.write(90);
  } else if (message == "CLOSE") {
      servoVentana.write(0);
  } else if (message == "ON") {
      servoVentilador.write(90);
  } else if (message == "OFF") {
      servoVentilador.write(0);
  } else {
      Serial.println("Error: Mensaxe non recoñecida");
  }

}

// Reconecta co broker MQTT se se perde a conexión
void reconnect() {
 while (!client.connected()) {
  Serial.print("Intentando conectar a broker MQTT...");

  if (client.connect("NAPIoT-P2-Vigo-Rec")) {
    Serial.println("conectado!");

    // Subscripción ao topic
    client.subscribe(mqtt_topic);
    Serial.println("Subscrito ao topic");
  } else {
    Serial.print("erro na conexión, erro=");
    Serial.print(client.state());
    Serial.println(" probando de novo en 5 segundos");
    delay(5000);
  }
 }
}

void setup() {
    if (!scd30.begin()) 
    {
        Serial.println("¡No se ha encontrado el sensor, comprueba las conexiones!");
        while (1);
    }

    // Configuración dos servos
    servoVentana.attach(servoVentanaPin);
    servoVentilador.attach(servoVentiladorPin);

    // Configuración do porto serie
    Serial.begin(115200);
    // Conexión coa WiFi
    setup_wifi();

    // Configuración de MQTT
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void readData(){
    Serial.println("Data available!");

    if (!scd30.read()){ Serial.println("Error reading sensor data"); return; }

    Serial.print("Temperature: ");
    Serial.print(scd30.temperature);
    Serial.println(" degrees C");
    
    Serial.print("Relative Humidity: ");
    Serial.print(scd30.relative_humidity);
    Serial.println(" %");
    
    Serial.print("CO2: ");
    Serial.print(scd30.CO2, 3);
    Serial.println(" ppm");
    Serial.println("");

    //Conversión de datos para o seu correcto envio
    dtostrf(scd30.CO2, 6, 2, CO2);      
    dtostrf(scd30.temperature, 6, 2, Temp); 

    client.publish("devices/NAPIoT-P2Vigo/CO2", CO2);
    client.publish("devices/NAPIoT-P2Vigo/Temp", Temp);
}

void loop() {

    if (!client.connected())
        reconnect();
    if (scd30.dataReady())
        readData();

    delay(5000);
    client.loop();
}
