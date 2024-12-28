#include <Arduino.h>

#define LED_PIN 2
// Declare queue handle
QueueHandle_t queue1;

// Task prototypes
void sender_task(void *pvParameters);
void receiver_task(void *pvParameters);
void Blink_task(void *pvParameter);

typedef enum{
  sender1,
  sender2
}IDSender;

uint32_t count = 100;
uint32_t buf;
void setup() {
  // Initialize Serial
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  // Create the queue with a size of 1 (can hold one 32-bit value at a time)
  queue1 = xQueueCreate(1, sizeof(uint32_t));
  
  if (queue1 == NULL) {
    Serial.println("Queue creation failed!");
    while (1); // Stop execution if queue creation fails
  }

  // Create tasks
  xTaskCreate(sender_task, "Sender Task", 1024, NULL, 2, NULL);
  xTaskCreate(receiver_task, "Receiver Task", 1024, NULL, 2, NULL);
  xTaskCreate(Blink_task, "Blink Task", 1024, NULL, 1, NULL);
 
  
}

void loop() {

}

void sender_task(void *pvParameters) {
  
  while (1) {
    // Send count to queue
    if (xQueueSend(queue1, &count, 0) == pdPASS) {
      Serial.print("Sent: ");
      Serial.println(count);
    }
    
    vTaskDelay(pdMS_TO_TICKS(100)); 
  }
}

void receiver_task(void *pvParameters) {
  
  while (1) {
    // Receive data from queue
    if (xQueueReceive(queue1, &buf, 0) == pdPASS) {
      Serial.print("Received: ");
      Serial.println(buf); 
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
    count += 100;
    if(count > 1000) count = 100;
  }
}

// Task to blink led
void Blink_task(void *pvParameter)
{
  static uint8_t state = 1;
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(buf));
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(buf));
  }
}