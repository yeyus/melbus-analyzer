/**
 * MELBUS ANALYZER
 * Jesus Trujillo <elyeyus@gmail.com>
 *
 * Pin Assignments
 *    MDATA - Pin 3
 *    MCLK - Pin 2
 *    MBUSY - Pin 4
 */

#define MDATA 3
#define MCLK 2
#define MBUSY 4

volatile byte mByte;
volatile unsigned int bitCount = 0;
unsigned int bps = 0;

volatile boolean prevMBUSYstatus = HIGH;
volatile boolean didMBUSYchange = false;
volatile boolean newByte = false;

void setup() 
{
  // Configure timer0
  cli();
  //set timer0 interrupt at 100Hz~
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 100Hz~ increments
  OCR0A = 155;// = (16*10^6) / (100*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 1024 prescaler
  TCCR0B |= (1 << CS02) | (0 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();
  
  // Initialize serial port
  Serial.begin(115200,SERIAL_8N1);
  
  pinMode(MBUSY,INPUT);
  attachInterrupt(0,melbus_clock_isr,FALLING);
}

void loop() 
{
  
  // Print the status of the MBUSY line when changes
  if(didMBUSYchange) {
    Serial.println("MBUSY: ");
    Serial.print(prevMBUSYstatus, BIN);
    didMBUSYchange = false;
  }
  
  // Print bus speed
  Serial.println("BPS: ");
  Serial.print(bps);
  
  // Print last byte
  Serial.println("DATA: ");
  Serial.print(mByte, HEX);
  newByte = false;
  
}

boolean melbus_is_busy()
{
  return digitalRead(MBUSY);
}

/**
 * melbus_clock_isr
 * Takes care of signaling the melbus clock so data bit capture could be performed
 */
void melbus_clock_isr() 
{
  // Track MBUSY line
  if(prevMBUSYstatus != melbus_is_busy()) {
    didMBUSYchange = true;
    prevMBUSYstatus = melbus_is_busy();
  }
  
  bitCount++;
  
  if(bitCount%8 == 0 && bitCount > 0) {
    newByte = true;
  }
  // push value of MDATA into mByte
  mByte = mByte<<1;
  mByte |= 0x01&digitalRead(MDATA); 
  
}

ISR(TIMER0_COMPA_vect)
{
    // reset bit count
    bps = bitCount*100;
    bitCount = 0;
}
