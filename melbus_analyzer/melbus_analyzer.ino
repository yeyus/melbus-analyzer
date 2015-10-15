/**
 * MELBUS ANALYZER
 * Jesus Trujillo <elyeyus@gmail.com>
 *
 * Pin Assignments
 *    MCLK - Pin 2 (PD2) (INT0) (PCINT18)
 *    MBUSY - Pin 3 (PD3) (INT1) (PCINT19)
 *    MDATA - Pin 4 (PD4) (T0) (PCINT20)
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define MCLK 2
#define MBUSY 3
#define MDATA 4

#define DEBUG 1

#ifdef DEBUG
#define DBG_INT0 5
#define DBG_INT1 6
#define DBG_TIM1 7
#endif

// melbus state
volatile uint8_t melbus_clock = 7;
volatile uint8_t melbus_data = 0;

void setup() 
{
  // Set up interrupts to read MELBUS
  cli();

  // Set up external interrupt to catch MBUSY and MCLK state changes
  EIMSK = 1<<INT1 | 1<<INT0;  // Enable INT0&INT1
  EICRA = 0<<ISC11 | 1<<ISC10 | 0<<ISC01 | 1<<ISC00; // Trigger INT0&INT1 on level change

  // Enable timer1 interrupt
  TIMSK1 |= (1 << TOIE1);
  sei();
  
  // Initialize serial port
  Serial.begin(115200,SERIAL_8N1);
  Serial.println("--- Melbus Analyzer ---");
  pinMode(MBUSY,INPUT);
  pinMode(MCLK,INPUT);
  pinMode(MDATA,INPUT);
  
  #ifdef DEBUG
  pinMode(DBG_INT0, OUTPUT);
  pinMode(DBG_INT1, OUTPUT);
  pinMode(DBG_TIM1, OUTPUT);
  digitalWrite(DBG_INT0, LOW);
  digitalWrite(DBG_INT1, LOW);
  digitalWrite(DBG_TIM1, LOW);
  #endif
}

void loop() 
{}

//MCLK change
ISR(INT0_vect) {
  // Preload timer will give around 30us timeout should be enough even with
  // slower clock coming from CD module
  TCNT1 = 0xFE1F;
  // Start timer without prescaler F_CPU/1
  TCCR1B |= (1 << CS10);

  #ifdef DEBUG
  digitalWrite(DBG_INT0, HIGH);
  #endif
  
  if (digitalRead(MCLK)) {
    if (digitalRead(MDATA)) {
      melbus_data |= (1 << melbus_clock);
    } else {
      melbus_data &= ~(1 << melbus_clock);
    }	
    
    melbus_clock--;
  }
  
  #ifdef DEBUG
  digitalWrite(DBG_INT0, LOW);
  #endif
}

// MBUSY change
ISR(INT1_vect) {
  #ifdef DEBUG
  digitalWrite(DBG_INT1, HIGH);
  #endif
  
  Serial.print("\n");
  
  #ifdef DEBUG
  digitalWrite(DBG_INT1, LOW);
  #endif
}

//Timer overflow 30us after last bit, it should clear the timer and mark the end of a byte
ISR(TIMER1_OVF_vect) {
  #ifdef DEBUG
  digitalWrite(DBG_TIM1, HIGH);
  #endif
  
  Serial.print(melbus_data, HEX);
  Serial.print(" ");
  melbus_clock = 7;

  //Stop timer
  TCCR1B &= ~(1 << CS10);
  
  #ifdef DEBUG
  digitalWrite(DBG_TIM1, LOW);
  #endif
}
