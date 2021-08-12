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
#define MSG_BUFFER_SIZE 238 // el maximo segun documentacion son 256 bytes (datos por paquete) pero unos se usan para headers y demas (238)
#define Packages 3000 // cantidad de paquetes necesarias para completar cerca de 8segundos
int i;
int k;
byte voice[MSG_BUFFER_SIZE];

//TaskHandle_t Task1;

//********** FUNCIONES SETUP *********************
//void callback(char* topic, byte* payload, unsigned int length);
//void reconnect();
//void setup_wifi();

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(led1, OUTPUT);
  //pinMode(portPin, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
 /* xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
           //         "Task1",     /* name of task. */
             //       10000,       /* Stack size of task */
               //     NULL,        /* parameter of the task */
                 //   1,           /* priority of the task */
                   // &Task1,      /* Task handle to keep track of created task */
                   // 0);          /* pin task to core 0 */                  
  /*delay(500); */
}

//**************LOOP***************
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  
  if(client.connected()){
    Serial.println("Empieza...");
    for(k= 0; k<Packages; k++){ //la cantidad de paquetes que se envian antes de la pausa. se requieren 4900 paquetes de 90 bytes para completar 10s con 44100 fs.
      for (i = 0; i < MSG_BUFFER_SIZE; i=i+2) {     // este for crea un paquete de 230 bytes
        int portValue = analogRead(portPin);   // los valores van de 0 a 4095, datos centrados en 2048 (int = 16 bits)
        portValue = (portValue*16);          //para aprovechar todo el int16 ya que de 0 a 4095 solo se aprovechan 10 bits
        /*int vall = portValue - 32768;     // esto era para verificar los niveles fueran los correctos
        if (vall > 32767){
          vall = 32767;
        }
        else if (vall < -32768){
          vall = -32768;
        }*/
        
        data0 = 0xff & portValue;           //Separa el int16 en dos bytes, parte inferior
        data1 = 0xff & (portValue >> 8);    //parte superior
        
        //voice[i] = portValue;                // se reduce el int (16 bits) a byte(8 bits), los valores van de 0 a 255, datos centrados en 127
        voice[i] = data0;
        voice[i+1] = data1;
        //Serial.println(vall);             // para comparar que se recibe el mismo dato enviado
        //client.write(voice[i]);
        
        delayMicroseconds(22); //44.1kHz - frecuencia de muestreo  
      }
      client.publish(root_topic_publish, voice, MSG_BUFFER_SIZE);    //se envia un paquete (voice) de 90bytes
      //Serial.println("Paquete enviado");
    }
    Serial.println("Audio enviado");
    Serial.println("Pausa...");
  }
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
