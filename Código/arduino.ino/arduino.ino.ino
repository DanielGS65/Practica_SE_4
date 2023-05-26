#include "DHT.h"
#define DHTPIN 2 //Esta conectado en el pin 2!
#define DHTTYPE DHT22   // Sensor DHT22
#include <WiFiNINA.h>
#include <PubSubClient.h>
#define LIGHTSENSORPIN A3 //Ambient light sensor reading
#define DEVICE_LABEL "arduino-nano-33"
#define TOKEN "BBFF-o63O7Ap7f07GwsPU6Jm4JWDUIF2vPI"

DHT dht(DHTPIN, DHTTYPE);

const int AirValue = 900;   //you need to replace this value with Value_1
const int WaterValue = 550;  //you need to replace this value with Value_2
int soilMoistureValue[3] = {0,0,0};
int soilmoisturepercent[3] = {0,0,0};
char const * SSID_NAME = "*******"; // Put here your SSID name
char const * SSID_PASS = "*******"; // Put here your password
char mqtt_ip[] = "169.55.61.243";
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void connectWifi(){
    Serial.println("Connecting to wifi");
    if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  int attempts = 0;
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SSID_NAME);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(SSID_NAME, SSID_PASS);
    // wait 10 seconds for connection:
    delay(2000);
    attempts++;
    if(attempts > 5){
      while(true){};
    }
  }
}

void connectMqtt(){

  client.setServer(mqtt_ip,1883);
  
  
  int attempts = 0;
  
  while(!client.connected()){
    client.connect("dgs65MQTT", TOKEN,"");
    delay(2000);
    attempts++;
    
    if(attempts > 5){
      while(true){};
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando el sensor de humedad");
  dht.begin(); //Iniciamos el sensor
  Serial.println("Se ha iniciado el sensor de humedad correctamente");
  pinMode(LIGHTSENSORPIN,  INPUT);
  connectWifi();
  connectMqtt(); 
}

void loop() {
 
  client.loop(); //Envio de mensajes 
  
  soilMoistureValue[0] = analogRead(A0);
  soilMoistureValue[1] = analogRead(A1);
  soilMoistureValue[2] = analogRead(A2);
  float h_air=dht.readHumidity();
  float t = dht.readTemperature(); //Leemos la temperatura en grados Celsius
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  float reading = analogRead(LIGHTSENSORPIN); //Read light level
  Serial.println(reading);
  float square_ratio = reading / 1023.0;      //Get percent of maximum value (1023)
  square_ratio = pow(square_ratio, 2.0);      //Square to make response more obvious
  

  for(int i=0; i<3; i++){
    soilmoisturepercent[i] = map(soilMoistureValue[i], AirValue, WaterValue, 0, 100);
    if(soilmoisturepercent[i] <= 0){soilmoisturepercent[i] = 0;}
    else if(soilmoisturepercent[i] >= 100){soilmoisturepercent[i] = 100;}
    Serial.println(soilmoisturepercent[i]);
    
  }

 
  char payload[8];
  String str_humedad = String(soilmoisturepercent[0]);
  str_humedad.toCharArray(payload, str_humedad.length()+1);
  client.publish("/v1.6/devices/arduino-nano-33/Humedad1",payload);
  Serial.println("Sended Hum0");
  delay(1000);
  
  str_humedad = String(soilmoisturepercent[1]);
  str_humedad.toCharArray(payload, str_humedad.length()+1);
  client.publish("/v1.6/devices/arduino-nano-33/Humedad2",payload);
  Serial.println("Sended Hum1");
  delay(1000);
  
  str_humedad = String(soilmoisturepercent[2]);
  str_humedad.toCharArray(payload, str_humedad.length()+1);
  client.publish("/v1.6/devices/arduino-nano-33/Humedad3",payload);
  Serial.println("Sended Hum2");
  delay(1000);
  
  str_humedad = String(t);
  str_humedad.toCharArray(payload, str_humedad.length()+1);
  client.publish("/v1.6/devices/arduino-nano-33/Temperatura",payload);
  Serial.println("Sended Temp");
  delay(1000);
  
  str_humedad = String(h_air);
  str_humedad.toCharArray(payload, str_humedad.length()+1);
  client.publish("/v1.6/devices/arduino-nano-33/HumedadAerea",payload);
  Serial.println("Sended Humair");
  delay(1000);
  
  str_humedad = String(square_ratio);
  str_humedad.toCharArray(payload, str_humedad.length()+1);
  client.publish("/v1.6/devices/arduino-nano-33/Luz",payload);
  Serial.println("Sended Light");
  delay(5000);
}
