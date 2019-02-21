#include <Arduino.h>

//#define I2C

#ifdef I2C
#define I2C_SLAVE_ADDRESS 0x4 // Address of the slave
#include <TinyWireS.h>

#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif 
#endif

#define SLEEP_PIN A3
#define WORK_PIN A2
#ifdef I2C
#define RELAY_PIN 1
#else
#define RELAY_PIN 0
#endif

const unsigned long MAX_SLEEP_TIME_SEC = 30 * 60UL;
const unsigned long MAX_WORK_TIME_SEC = 30 * 60UL;
const unsigned long CHECK_PERIOD_MS = 1000;

struct INFO
{
  bool working;
  unsigned long mode_start;

  int sleepSense;
  int workSense;
  unsigned int sleepTimeSec;
  unsigned int workTimeSec;
  unsigned long duration;
};

INFO info;

union BUFFER
{
   INFO* info;
   byte* bytes;
};

BUFFER buffer;
unsigned int read_pos;
const unsigned int buff_size = sizeof(INFO);

// #define SEND_INT(val) \
//   TinyWireS.send(val & 0xFF); \
//   TinyWireS.send((val >> 8) & 0xFF); 

// #define SEND_LONG(val) \
//   TinyWireS.send(val & 0xFF); \
//   TinyWireS.send((val >> 8) & 0xFF); \
//   TinyWireS.send((val >> 16) & 0xFF); \
//   TinyWireS.send((val >> 24) & 0xFF); 

#ifdef I2C

void requestEvent()
{
    //TinyWireS.send(buffer.bytes[read_pos++]);
    // for(unsigned int i=0;i<buff_size;i++)
    // {
    //   TinyWireS.send(i);
    // }
    TinyWireS.send(buffer.bytes[read_pos++]);
    if(read_pos >= buff_size)
    {
      read_pos = 0;
    }   
}
#endif

void setup() {
  buffer.info = &info;
  read_pos = 0;

#ifdef I2C
  TinyWireS.begin(I2C_SLAVE_ADDRESS); // join i2c network
    //TinyWireS.onReceive(receiveEvent); // not using this
   TinyWireS.onRequest(requestEvent);
  
#endif
  //pinMode(SLEEP_PIN, INPUT);
  //pinMode(WORK_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);

  info.working = true;
  info.mode_start = millis();
}

void flip()
{
  info.working = !info.working;
  info.mode_start = millis();
   
  //inverted Relay logic
  digitalWrite(RELAY_PIN, info.working ? LOW : HIGH);
}

void loop()
{
  info.sleepSense = analogRead(SLEEP_PIN);
  info.workSense = analogRead(WORK_PIN);
  
  info.sleepTimeSec = map(info.sleepSense, 0, 1023, 1, MAX_SLEEP_TIME_SEC);
  info.workTimeSec = map(info.workSense, 0, 1023, 1, MAX_WORK_TIME_SEC);

  //sleepTimeSec = workTimeSec = 15*60;

  info.duration = millis() - info.mode_start;
 

  if(info.working)
  {
    if( info.duration >= info.workTimeSec * 1000UL)
    {
      flip();
    }
  }
  else
  {
    if( info.duration >= info.sleepTimeSec * 1000UL)
    {
      flip();
    }
  }  

  //tws_delay(CHECK_PERIOD_MS);

#ifdef I2C
  TinyWireS_stop_check();
#endif

  
}

