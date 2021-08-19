#include <driver/i2s.h>   //I2C interface

#define I2S_WS 15     //
#define I2S_SD 13
#define I2S_SCK 2
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (16000)

void setup() {
  Serial.begin(115200);
  Serial.println("Setup I2S ...");

  delay(1000);
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  delay(500);
}

void loop() {
  // Read a single sample and log it for the Serial Plotter.
  int32_t sample = 0;
  int bytes = i2s_pop_sample(I2S_PORT, (char*)&sample, portMAX_DELAY);      //Pop (read) a single sample from the I2S DMA RX buffer. (portMAX_DELAY = no timeout) 
  if(bytes > 0){
    Serial.println(sample);
  }
}

void i2s_install(){
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),    //  Se elige el modo RX que es receptor, por defecto es TX, transmisor
    .sample_rate = I2S_SAMPLE_RATE,     //Sample rate los 44.1kHz que piede la IA
    .bits_per_sample = i2s_bits_per_sample_t(32),        // Deberia ser 16 bits pero eso esta bugeado, toca usar 32 bits
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,         // I had to use I2S_CHANNEL_FMT_ONLY_LEFT whenever L/R pin was unconnected/ground
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 8,      // cantidad de buffers, minimo 2
    .dma_buf_len = 64,      // entre 1024 - 8. Tamaño del buffer
    .use_apll = false       // si es "true", se usa APLL-CLK source que tiene freq entre 16-128MHz
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);   //Se instala el driver con la configuracion
}

void i2s_setpin(){
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,        // Serial Clock
    .ws_io_num = I2S_WS,          // Word Select
    .data_out_num = -1,           // para uso de parlantes
    .data_in_num = I2S_SD         // Serial Data
  };

  i2s_set_pin(I2S_PORT, &pin_config);     // Se añade la configuracion de pines
}
