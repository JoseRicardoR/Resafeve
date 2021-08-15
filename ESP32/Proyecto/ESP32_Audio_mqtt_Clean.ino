//#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//*********** MQTT CONFIG **************
const char *mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char *mqtt_user = "";
const char *mqtt_pass = "";
const char *root_topic_subscribe = "raiz/input";
const char *root_topic_publish = "raiz/output";

//*********** WIFICONFIG ***************
const char* ssid = "FAMILIA RINCON";
const char* password =  "Alana4545";

//*********** GLOBALES   ***************
WiFiClient espClient;
PubSubClient client(espClient);
//char msg[25];
long count=0;
const int led1 = 2;
const int portPin = 34;
int portValue = 0;
byte data0 = 0;
byte data1 = 0;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE 48000 // el maximo segun documentacion son 256 bytes (datos por paquete) pero unos se usan para headers y demas (238)
#define Packages 3 // cantidad de paquetes necesarias para completar cerca de 8.5 segundos
#define SAMPLING_FREQUENCY 8000
int i;
int k;
byte voice[MSG_BUFFER_SIZE];


unsigned long tiempoEpack = 0;
unsigned long tiempoFpack = 0;
unsigned long tiempoEmess = 0;
unsigned long tiempoFmess = 0;
unsigned long tiempoEsamp = 0;
unsigned long tiempoFsamp = 0;
unsigned long diferenciaTiempoPack = 0;
unsigned long diferenciaTiempoMess = 0;
unsigned long diferenciaTiempoSamp = 0;
unsigned int sampling_period_us;

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(led1, OUTPUT);
  //pinMode(portPin, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(48500);
  Serial.println(client.getBufferSize());
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  //analogSetCycles(4);  // Set number of cycles per sample, default is 8 and provides an optimal result, range is 1 - 255
  delay(1000);
}

//**************LOOP***************
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  
  if(client.connected()){
    
    Serial.println("Empieza...");    
    tiempoEmess = millis();
    //client.beginPublish(root_topic_publish, MSG_BUFFER_SIZE*Packages, false);
    
    for(k= 0; k<Packages; k++){   //for que graba x paquetes....
    tiempoEpack = millis();
    
      for (i = 0; i < MSG_BUFFER_SIZE; i=i+2) {     //for que graba el cada paquete......
        tiempoEsamp = micros();
        
        int portValue = analogRead(portPin);   // los valores van de 0 a 4095, datos centrados en 2048 (int = 16 bits) 
        portValue = (portValue*16);          //para aprovechar todo el int16 ya que de 0 a 4095 solo se aprovechan 12 bits 
    
        data0 = 0xff & portValue;           //Separa el int16 en dos bytes, parte inferior
        data1 = 0xff & (portValue >> 8);    //parte superior
        voice[i] = data0;
        voice[i+1] = data1;
        
        while ((micros() - tiempoEsamp) < sampling_period_us) { /* do nothing to wait */ } // espera se cumpla el tiempo entre muestras
        /*tiempoFsamp = micros();
        diferenciaTiempoSamp = tiempoFsamp - tiempoEsamp;
        Serial.println(diferenciaTiempoSamp);*/
      }

      client.publish(root_topic_publish, voice, MSG_BUFFER_SIZE);
      //client.write(voice, MSG_BUFFER_SIZE);
      tiempoFpack = millis(); 
      diferenciaTiempoPack = tiempoFpack - tiempoEpack;
      Serial.println(diferenciaTiempoPack);
    }

    //client.endPublish()
    Serial.println("Audio enviado");
    tiempoFmess = millis();
    diferenciaTiempoMess = tiempoFmess - tiempoEmess;
    Serial.println(diferenciaTiempoMess);
    
  }
  Serial.println("Pausa...");
  delay(30000);  // Deja de enviar audio por 30s
  
  client.loop(); 
}


//************ FUNCIONES WIFI ******************************
//***CONEXION WIFI*****
void setup_wifi(){
  delay(10);
  // Nos conectamos a nuestra red Wifi
  Serial.println();
  Serial.print("Conectando a ssid: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a red WiFi!");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}
//**CONNECT MQTT******
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión Mqtt...");
    // Creamos un cliente ID
    String clientId = "ESP_32_Mic1";
    clientId += String(random(0xffff), HEX);
    // Intentamos conectar
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
      // Nos suscribimos
      if(client.subscribe(root_topic_subscribe)){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
    } else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
    }
  }
}
//*** CALLBACK recibir mensajes ****
void callback(char* topic, byte* payload, unsigned int length){ 
    /*---------------mensaje recibido-------------*/        
  String incoming = "";
  Serial.print("Mensaje recibido desde -> ");
  Serial.print(topic);
  Serial.println("");
  for (int i = 0; i < length; i++) {
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje -> " + incoming);
  
  if(incoming == "LED"){
    Serial.println("Deberia prender...");
    digitalWrite(led1, HIGH);
    delay(5000);
    digitalWrite(led1, LOW);
    delay(1000);
  }  
}
