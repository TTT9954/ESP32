/**
 * 
 * One task performs basic echo on Serial. If it sees "delay" followed by a
 * number, it sends the number (in a queue) to the second task. If it receives
 * a message in a second queue, it prints it to the console. The second task
 * blinks an LED. When it gets a message from the first queue (number), it
 * updates the blink delay to that number. Whenever the LED blinks 100 times,
 * the second task sends a message to the first task to be printed.
 * 
 */

#include <Arduino.h>

// Use 1 core 
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define BUF_LEN           255
#define MSG_QUEUE_LEN     5
#define DELAY_QUEUE_LEN   5
#define BLINK_MAX         100
#define LED_PIN           LED_BUILTIN

static const char command[] = "delay "; // Note the space!


// Message struct: used to wrap strings (not necessary, but it's useful to see
// how to use structs here)
typedef struct Message {
  char msg[20];
  int count;
} Message;

// Globals
static QueueHandle_t delay_queue;
static QueueHandle_t msg_queue;

void CommandProcess(void *parameters); // Task: command line interface (CLI)
void blinkLED(void *parameters); // Task: flash LED based on delay provided, notify other task every 100 blinks


//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Configure Serial
  Serial.begin(115200);

  
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait a moment to start
  Serial.println();
  Serial.println("Enter the command 'delay xxx' where xxx is your desired ");
  Serial.println("LED blink delay time in milliseconds");

  // Create queues
  delay_queue = xQueueCreate(DELAY_QUEUE_LEN, sizeof(int));
  msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(Message));

  // Start CLI task
  xTaskCreatePinnedToCore(CommandProcess,
                          "CLI",
                          2048,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Start blink task
  xTaskCreatePinnedToCore(blinkLED,
                          "Blink LED",
                          2048,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}

// Task: command line interface (CLI)
void CommandProcess(void *parameters) {

  Message rcv_msg;
  char c;
  char buf[BUF_LEN];
  uint8_t idx = 0;
  uint8_t cmd_len = strlen(command);
  int led_delay;

  // Clear whole buffer
  memset(buf, 0, BUF_LEN);

  // Loop forever
  while (1) {

    // See if there's a message in the queue (do not block)
    if (xQueueReceive(msg_queue, (void *)&rcv_msg, 0) == pdTRUE) {
      Serial.print(rcv_msg.msg);
      Serial.println(rcv_msg.count);
    }

    // Read characters from serial
    if (Serial.available() > 0) {
      c = Serial.read();

      // Store received character to buffer if not over buffer limit
      if (idx < BUF_LEN - 1) {
        buf[idx] = c;
        idx++ ;
      }

      // Print newline and check input on 'enter'
      if ((c == '\n') || (c == '\r')) {

        // Print newline to terminal
        Serial.print("\r\n");
        Serial.println(buf);
        // Check if the first 6 characters are "delay "
        if (memcmp(buf, command, cmd_len) == 0) {

          // Convert last part to positive integer (negative int crashes)
          char* tail = (buf+cmd_len);
          led_delay = atoi(tail);
          led_delay = abs(led_delay);

          // Send integer to other task via queue
          if (xQueueSend(delay_queue, (void *)&led_delay, 10) != pdTRUE) {
            Serial.println("ERROR: Could not put item on delay queue.");
          }
        }

        // Reset receive buffer and index counter
        memset(buf, 0, BUF_LEN);
        idx = 0;

      // Otherwise, echo character back to serial terminal
      } else {
        Serial.print(c);
      }
    } 
  }
}

// Task: flash LED based on delay provided, notify other task every 100 blinks
void blinkLED(void *parameters) {

  Message msg;
  int led_delay = 500;
  uint8_t counter = 0;

  // Set up pin
  pinMode(LED_BUILTIN, OUTPUT);

  // Loop forever
  while (1) {

    // See if there's a message in the queue (do not block)
    if (xQueueReceive(delay_queue, (void *)&led_delay, 0) == pdTRUE) {

      // Best practice: use only one task to manage serial comms
      strcpy(msg.msg, "Message received ");
      msg.count = 1;
      xQueueSend(msg_queue, (void *)&msg, 10);
    }

    // Blink
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);

    // If we've blinked 100 times, send a message to the other task
    counter++;
    if (counter >= BLINK_MAX) {
      
      // Construct message and send
      strcpy(msg.msg, "Blinked: ");
      msg.count = counter;
      xQueueSend(msg_queue, (void *)&msg, 10);

      // Reset counter
      counter = 0;
    }
  }
}