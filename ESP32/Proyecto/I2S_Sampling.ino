#include <driver/i2s.h>   //I2S interface
#include <WiFi.h>         // Wifi Library
#include <PubSubClient.h>   // MQTT Library

//*********** WIFI CONFIG ***************
const char* ssid = "FAMILIA RINCON";
const char* password =  "Alana4545";
WiFiClient espClient;

//***********FreeRTOS***************
TaskHandle_t RecordAudioTask;

//*********** MQTT CONFIG **************
PubSubClient client(espClient);
const char *mqtt_server = "3.16.69.93";
const int mqtt_port = 1883;
const char *mqtt_user = "";
const char *mqtt_pass = "";
const char *root_topic_subscribe = "ESP32/Signal_input";
const char *root_topic_publish = "ESP32/Data_output";

//********** I2S CONFIG *******************
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (16000)     //frecuencia de muestreo
#define I2S_SAMPLE_BITS (16)        //Deberia ser 16 bits pero eso esta bugeado, toca usar 32 bits
#define TIME (6)
#define FLASH_RECORD_SIZE (1 * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * TIME) //#channels * fs * SampBits / 8 * t(seg)
#define I2S_READ_LEN (24000)

const int headerSize = 44;
//***** Cronometros ****************
unsigned long tiempoEmess = 0;
unsigned long tiempoFmess = 0;
unsigned long diferenciaTiempoMess = 0;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP32 encendidio...");
  Serial.println("Setup I2S ...");
  i2s_init();
  Serial.println("Setup WIFI ...");
  setup_wifi();
  Serial.println("Setup MQTT ...");
  client.setServer(mqtt_server, mqtt_port);
  MQTT_connect();
  client.setCallback(MQTT_callback);
  client.setBufferSize(48500);
  xTaskCreate(i2s_adc, "i2s_adc", 2 * 1024, NULL, 1, &RecordAudioTask);
  delay(5000);
}

void loop() {
  // Todo lo controla FreeRTOS
}

//***************** TAREA FREERTOS ********************************
//*** RECORD ADC-I2S ***
void i2s_adc(void *arg)
{
  /*byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);
  client.publish(root_topic_publish, header, headerSize);*/

  int i2s_read_len = I2S_READ_LEN;
  int flash_wr_size = 0;
  size_t bytes_read;

  char* i2s_read_buff = (char*) calloc(i2s_read_len, sizeof(char));       // Buffer de lectura del I2S
  uint8_t* flash_write_buff = (uint8_t*) calloc(i2s_read_len, sizeof(char));   //buffer de escritura (flash memory)

  //i2s_adc_enable(I2S_NUM);        //Start to use I2S built-in ADC mode. (less noise)
  //i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);  //Tal parece que le toma tiempo al I2S iniciarse entonces
  tiempoEmess = millis();
  Serial.println(" ... Recording Start ... ");
  while (flash_wr_size < FLASH_RECORD_SIZE) {
    //read data from I2S bus, in this case, from ADC.

    esp_err_t err = i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    example_disp_buf((uint8_t*) i2s_read_buff, 64);
    if (err != ESP_OK) {
      Serial.println("Error while recording!");
      break;
    }
    //save original data from I2S(ADC) into flash.
    i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, i2s_read_len);

    client.publish(root_topic_publish, (uint8_t*)flash_write_buff, i2s_read_len);

    flash_wr_size += i2s_read_len;
    Serial.println("... Package sent ...");

    ets_printf("Sound recording %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
    ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
  }
  tiempoFmess = millis();
  Serial.println(" ... Recording Finish ... ");
  //i2s_adc_disable(I2S_NUM);        //Stop to use I2S built-in ADC mode

  diferenciaTiempoMess = tiempoFmess - tiempoEmess;
  Serial.println("Tiempo total: ");
  Serial.println(diferenciaTiempoMess);

  free(i2s_read_buff);
  i2s_read_buff = NULL;
  free(flash_write_buff);
  flash_write_buff = NULL;

  Serial.println("Pausa...");
  delay(30000);  // Deja de enviar audio por 30s
  
  client.loop(); 
  
  //vTaskDelete(NULL);
}



//********************** FUNCIONES I2C ****************************
//*** CONFIGURACION, INSTALACION y SELECCION DE PINES ***
void i2s_init() {
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX ),    //  Se elige el modo RX que es receptor, por defecto es TX, transmisor
    .sample_rate = I2S_SAMPLE_RATE,     //Sample rate los 44.1kHz que piede la IA
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),        // Deberia ser 16 bits pero eso esta bugeado
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,         // I had to use I2S_CHANNEL_FMT_ONLY_LEFT whenever L/R pin was unconnected/ground
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 20,      // cantidad de buffers, minimo 2
    .dma_buf_len = 1000,      // entre 1024 - 8. Tamaño del buffer
    .use_apll = 0       // si es "true", se usa APLL-CLK source que tiene freq entre 16-128MHz
  };
  esp_err_t errIns = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL); //Se instala el driver con la configuracion
  if (errIns != ESP_OK)
  {
    Serial.printf("Failed installing the driver: %d\n", errIns);
  } else {
    Serial.println("I2S driver installed");
  }

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,        // Serial Clock
    .ws_io_num = I2S_WS,          // Word Select
    .data_out_num = -1,           // para uso de parlantes
    .data_in_num = I2S_SD         // Serial Data
  };
  esp_err_t errPin = i2s_set_pin(I2S_PORT, &pin_config);     // Se añade la configuracion de pines
  if (errPin != ESP_OK)
  {
    Serial.printf("Failed setting pin: %d\n", errPin);
  } else {
    Serial.println("I2S Pins set");
  }
}
//*** SCALE DATA I2S ***
void i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len)
{ //se escala de la info para la salida
  uint32_t j = 0;
  uint32_t dac_value = 0;
  for (int i = 0; i < len; i += 2) {
    dac_value = ((((uint16_t) (s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
    d_buff[j++] = 0;
    d_buff[j++] = dac_value * 256 / 2048 ;
  }
}
//***wav Header**
/*void wavHeader(byte* header, int wavSize) {
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSize = wavSize + headerSize - 8;
  header[4] = (byte)(fileSize & 0xFF);          //Audio size..
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF); //..
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;  // Resolucion en bits..
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00; //..
  header[20] = 0x01; //Audio format= 1(PCM) ..
  header[21] = 0x00; //..
  header[22] = 0x01; // numero de canales ..
  header[23] = 0x00; //..
  header[24] = 0x80; //Samplerate ..
  header[25] = 0x3E; 
  header[26] = 0x00;
  header[27] = 0x00; //..
  header[28] = 0x00; //Byte Rate .. SampleRate * NumChannels * BitsPerSample/8
  header[29] = 0x7D;
  header[30] = 0x00;
  header[31] = 0x00; //..
  header[32] = 0x02; //Block align .. NumChannels * BitsPerSample/8
  header[33] = 0x00; //..
  header[34] = 0x10; //BitsPerSample
  header[35] = 0x00; //..
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(wavSize & 0xFF);        //Audio size..
  header[41] = (byte)((wavSize >> 8) & 0xFF);
  header[42] = (byte)((wavSize >> 16) & 0xFF);
  header[43] = (byte)((wavSize >> 24) & 0xFF);//..
}*/
//***PRINT BUFFER ***
void example_disp_buf(uint8_t* buf, int length)
{
  printf("======\n");
  for (int i = 0; i < length; i++) {
    printf("%02x ", buf[i]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
  printf("======\n");
}

//************ FUNCIONES WIFI ******************************
//***CONEXION WIFI***
void setup_wifi() {
  delay(10);
  // Nos conectamos a nuestra red Wifi
  Serial.print("Conectando a ssid: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);     //funcion que conecta
  while (WiFi.status() != WL_CONNECTED) {     //mientras conecta imprime . indicando espera
    delay(500);
    Serial.print(".");
  }
  Serial.println("");       //si llego aca es porque ya esta conectado
  Serial.println("Conectado a red WiFi!");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

//***************** FUNCIONES MQTT *******************
//***CONNECT MQTT***
void MQTT_connect() {
  while (!client.connected()) {         //si no esta conectado hace todo el proceso
    Serial.print("Intentando conexión Mqtt...");
    String clientId = "ESP_32_Mic1";
    clientId += String(random(0xffff), HEX);        // Creamos un cliente ID
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) { // Intentamos conectar
      Serial.println("Conectado!");
      if (client.subscribe(root_topic_subscribe)) {   // Nos suscribimos
        Serial.println("Suscripcion ok");
      } else {
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
//*** CALLBACK ***
void MQTT_callback(char* topic, byte* payload, unsigned int length) {   //se llama automaticamente con un mensaje recibido en subscribe topic
  String incoming = "";
  Serial.print("Mensaje recibido desde -> ");
  Serial.print(topic);    //topic al que esta conectado
  Serial.println("");
  for (int i = 0; i < length; i++) {
    incoming += (char)payload[i];     // concatenacion del mensaje
  }
  incoming.trim();                //elimina espacios en blanco en el mensaje
  Serial.println("Mensaje -> " + incoming);
}
