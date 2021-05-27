/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

TaskHandle_t Task1;
TaskHandle_t Task2;

// LED pins
const int led1 = 2;
const int potPin = 34;

// variable for storing the potentiometer value
int potValue = 0;

void setup() {
  Serial.begin(115200); 
  pinMode(led1, OUTPUT);
  //pinMode(potPin, INPUT);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 
}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    digitalWrite(led1, HIGH);
    delay(1000);
    digitalWrite(led1, LOW);
    delay(1000);
  } 
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  
  for(;;){
  potValue = analogRead(potPin);
  Serial.print("0, 4095, 2048, ");  // Son lis limites y el punto medio para cuadrar el ruido
  Serial.println(potValue);
  delay(10);

//Como realmente debe ser el proceso
  /*
   * 
   * Udp.beginPacket("192.168.1.101", 1234);    //  Se crea el paquete Udp que se va a enviar
   * for(int i=0; i<4095; i++){   //   El tamaño de los paquetes es 4095 bytes
    int old=micros();       

    float analog = analogRead(potPin);   // Entrada de datos

    analog = ((analog * 16) - 0);     /// Se divide o multiplica segun la sensibilidad requerida (para 4096 la resolucion es 12 bist, se quiere 16). Se resta si se quiere recentrar
    if (analog > 65536){     //   Por si acaso ponemos limites
      analog = 65536;
    }
    else if (analog < 0){      // Tambein inferior
      analog = 0;
    }
    
    Udp.write(int(analog));   // Se genera el paquete y se convierte a int ya que era float
    while(micros()-old<20); // 23uSec = 1Sec/44100Hz - 3uSec para los otros procesos ----  Esto es para que quede uestreado a la frecuencia seleccionada
  }
  Udp.endPacket();    // Se cierra el paquete
  delay(15000);         // delay entre creacion de cada paquete--- en este caso cada 15s
   */

  
  }
}

void loop() {
  
}
