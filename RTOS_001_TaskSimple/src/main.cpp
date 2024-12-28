#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// LED rates
static const uint16_t rate_1 = 100;  // ms
static const uint16_t rate_2 = 50;  // ms

// Pins
static const int led_pin = LED_BUILTIN;
uint8_t state = 0;
char msg[37] = "Hello Everbody, Today I feel so good";
char c = 'a';
uint16_t count;
/* Port prototype */
void Task1(void *parameter); // Task 1 function
void Task2(void *parameter); // Task 2 function
void ToggleLed(uint8_t pin); //  Function to toggle led
void TaskPrint(const char *msg, uint16_t rate); // Print each character of any message to serial monitor with the given time delay

void setup() {
  // Configure pin
  Serial.begin(115200);
  // pinMode(led_pin, OUTPUT);

  queue1 = xQueueCreate(10,sizeof(uint32_t)); // Creating a Queue which has length is 1 and size is sizeof(uint32_t)
  if(queue1 == 0)
  {
    Serial.print("Unable to creat Queue !\n ");
  }
  // Task to run forever
  xTaskCreate(  // Use xTaskCreate() in vanilla FreeRTOS
              Task1,  // Function to be called
              "Task1",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              &task1_handle         // Task handle
              ); 
      

  xTaskCreate(  // Use xTaskCreate() in vanilla FreeRTOS
              Task2,  // Function to be called
              "Task2",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );     //
  
}

void loop(){

}

/* Funtion to Toggle led */
void ToggleLed(uint8_t pin)
{
  state = ~state;
  digitalWrite(pin, state);
}

void TaskPrint(const char *msg, uint16_t rate)
{
  const char* msg_temp = msg;
  uint8_t length_msg = strlen(msg_temp);
  for(uint8_t i = 0; i < length_msg; i++)
  {
    Serial.print(msg_temp[i]);
    vTaskDelay(pdMS_TO_TICKS(rate));
  }
  c = msg_temp[length_msg - 1];
}

/* Task1 function definition */
void Task1(void *parameter) {
  char *param_temp = msg;
  while(1) {
    Serial.print('\n');
    TaskPrint(param_temp, rate_1);
    Serial.print('\n');
    Serial.print(c);
    Serial.print('\n');
  }
}

/* Task2 function definition */
void Task2(void *parameter) {
  while(1) {
    Serial.print('*');
    vTaskDelay(pdMS_TO_TICKS(rate_2));
  }
}

