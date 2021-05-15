#include <Arduino_FreeRTOS.h>

int led2=7;
int led3=8;
int led4=9;

int pot1=A0;
int pot2=A1;

void TaskBlink1( void *pvParameters );
void TaskBlink2( void *pvParameters );
void TaskBlink3( void *pvParameters );
void TaskBlink4( void *pvParameters );

void TaskAnalogRead1( void *pvParameters );
void TaskAnalogRead2( void *pvParameters );

void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink1
    ,  "Blink1"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
    
    xTaskCreate (
    TaskBlink2
    ,  "Blink2"  
    ,  128
    ,  NULL
    ,  3
    ,  NULL );

    xTaskCreate (
    TaskBlink3
    ,  "Blink3"  
    ,  128
    ,  NULL
    ,  3
    ,  NULL );

    xTaskCreate (
    TaskBlink4
    ,  "Blink4"  
    ,  128
    ,  NULL
    ,  3
    ,  NULL );
    
    
  xTaskCreate(
    TaskAnalogRead1
    ,  "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

    xTaskCreate(
    TaskAnalogRead2
    ,  "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}


void loop() {
  // put your main code here, to run repeatedly:

}

void TaskBlink1(void *pvParameters)  // This is a task.
{
  blink_my_led(LED_BUILTIN,100,pvParameters);
}


void TaskBlink2(void *pvParameters)  // This is a task.
{
  blink_my_led(led2,250,pvParameters);
}


void TaskBlink3(void *pvParameters)  // This is a task.
{
  blink_my_led(led3,500,pvParameters);
}


void TaskBlink4(void *pvParameters)  // This is a task.
{
  blink_my_led(led4,1000,pvParameters);
}



void TaskAnalogRead1(void *pvParameters)  // This is a task.
{
  myAnalogRead(pot1,"pot1:",pvParameters);
}


void TaskAnalogRead2(void *pvParameters)  // This is a task.
{
  myAnalogRead(pot2,"pot2:",pvParameters);
}

void blink_my_led(int led,int T,void *pvParameters)
{
  (void) pvParameters;
  pinMode(led, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( T / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( T / portTICK_PERIOD_MS ); // wait for one second
  }
}

  
void myAnalogRead(int pin, String my_name,void *pvParameters)
{
  (void) pvParameters;
 for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(pin);
    // print out the value you read:
    Serial.print(my_name);
    Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}
