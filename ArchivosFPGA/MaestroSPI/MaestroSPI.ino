#include <SPI.h>

//-- Pin usado para la seleccion del esclavo
#define SS 10

//-- Codigo de los comandos
#define SAP 0x7D  //-- Comando SET ADDRESS POINTER
#define WR  0x7E  //-- Comando de escritura en registro
#define RD  0x7F  //-- Comando de lectura en registro

//-- Direcciones de los registros
#define M16_1 0x01  //-- (Byte mas significativo de la muestra)
#define M16_2 0x02  //-- (Byte menos significativo de la muestra)
#define Comparacion 0x03  //-- (Emite un pulso que luego debera ser bajado a 0, este pulso da la orden de comparar la muestra con el umbral y contabilizar)
#define U_Reset 0x04  //-- (Resetea los contadores, hace la funcion de reset universal)
#define CriterioAudt 0x05  //-- (Informa si las muestras que se han analizado superan el umbral de audio valido)
#define LCompleta 0x06  //-- (Informa si ya se ha hecho la lectura de todas las muestras)
#define Ndato1 0x07  //-- (Recoge los 8 bits menos significativos de la cantidad total de datos que han sido leidos)
#define Ndato2 0x08  //-- (Recoge los segundos 8 bits menos significativos de la cantidad total de datos que han sido leidos)
#define Ndato3 0x09  //-- (Recoge el bit mas signicado de la cuenta total de datos, este bit sera el menos significativo del arreglo de 8 bits que se envia)
#define CUmbral1 0x0A  //-- (Recoge los 8 bits menos significativos de la cantidad total de muestras que han superado el umbral)
#define CUmbral2 0x0B  //-- (Recoge los segundos 8 bits menos significativos de la cantidad total de muestras que han superado el umbral)
#define CUmbral3 0x0C  //-- (Recoge el bit mas significativo de la cantidad total de muestras que han superado el umbral,este bit sera el menos significativo del arreglo de 8 bits que se envia) 
void setup() {

  //-- Inicializar SPI
  SPI.begin();
  SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
}

//-- Comando generico
uint8_t cmd(uint8_t cod, uint8_t val)
{
  digitalWrite(SS, LOW);

  //-- Enviar el codigo de comando
  SPI.transfer(cod);

  //-- Enviar el valor de su parametro
  uint8_t ret = SPI.transfer(val);
   
  digitalWrite(SS, HIGH);

  return ret;
}


void SAP_cmd(uint8_t value)
{
  cmd(SAP, value);
}

void WR_cmd(uint8_t value)
{
  cmd(WR, value);
}

uint8_t RD_cmd()
{
  return cmd(RD, 0x00);
}

//-- Escritura en un registro mapeado
void write_reg(uint8_t reg, uint8_t val)
{
  SAP_cmd(reg);
  WR_cmd(val);
}

//-- Lectura de un registro mapeado
uint8_t read_reg(uint8_t reg)
{
  SAP_cmd(reg);
  return RD_cmd();
}

uint8_t vgaleds = 0x01;

void loop() 
{
// Se mostrará un ejemplo del funcionamiento de las funciones
for(int i=0; i<16;i++){ // Se enviaran muestras de audio, se compararan con el umbral y el esclavo mostrará los valores de muestra que el recibió un total de 16 veces
  //-- Escribir la primera parte de la muestra (A2 por ejemplo)
  write_reg(M16_2,0xA2);
  //-- Escribir la segunda parte de la muestra (1B por ejemplo)
  write_reg(M16_2,0x1B);
  //-- Leer la muestra que se mandó
  int lectura=read_reg(M16_2)+read_reg(M16_1)*10^8;
  Serial.print("Muestra: ");
  Serial.println(lectura, HEX);
  //-- Esperar
  delay(500);
  //-- Comparar con el umbral 
  write_reg(Comparacion,01);
  write_reg(Comparacion,00);
//-- Escribir la primera parte de la muestra (A2 por ejemplo)
  write_reg(M16_2,0xA2);
  //-- Escribir la segunda parte de la muestra (FB por ejemplo)
  write_reg(M16_2,0xFB);
  //-- Leer la muestra que se mandó
  lectura=read_reg(M16_2)+read_reg(M16_1)*10^8;
  Serial.print("Muestra: ");
  Serial.println(lectura, HEX);
  //-- Esperar
  delay(500);
  //-- Comparar con el umbral 
  write_reg(Comparacion,0x01);
  write_reg(Comparacion,0x00);
}
int Datosleidos=read_reg(Ndato1); // Se captura el numero de datos comparados por el esclavo
Serial.print("#Datos leidos: ");
  Serial.println(Datosleidos, DEC);
  //-- Esperar
  delay(500);
int DatosMayores=read_reg(CUmbral1); // Se captura el numero de datos que han superado el umbral
Serial.print("#Datos que han superado el umbral: ");
  Serial.println(DatosMayores, DEC);
  //-- Esperar
  delay(500);
// Se limpian los registros para empezar un nuevo ciclo de comparación
write_reg(U_Reset,0x01);
write_reg(U_Reset,0x00);
 }
