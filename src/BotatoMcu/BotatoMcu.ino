#include "BotatoMcu.h"
#include <Wire.h>
#include <Servo.h> 

using namespace BotatoMcu;

Servo servo0;
bool newCmdArrived = false;

McuCommand cmd;

enum Pins {
  SERVO0_PIN = 8,
  ULTRASONIC0_TRIG_PIN = 10,
  ULTRASONIC0_ECHO_PIN = 9
};

class AutoLed
{
  public:
    AutoLed ()
    {
      digitalWrite(LED_BUILTIN, HIGH);
    }

    ~AutoLed ()
    {
      digitalWrite(LED_BUILTIN, LOW);
    }
};

void setup()
{
  // join i2c bus with specific address
  Wire.begin(MCU_SLAVE_ADDR);

  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  servo0.attach(SERVO0_PIN);
  servo0.write(0);

  Serial.begin(115200);
  Serial.println("BotatoRT MCU Startup");
}

void loop()
{
  if (newCmdArrived) {
      switch (cmd.CmdId) {
      case MCU_CMD_SERVO_WRITE:
        servo0.write(cmd.u.ServoWrite.Value);
        delay(15);
        break;
        
      case MCU_CMD_ULTRASONIC_READ:
        break;
    }
    newCmdArrived = false;
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{
  AutoLed led;
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int count)
{
  AutoLed led;

  // first byte is cmd
  uint8_t c = (uint8_t)Wire.read();
  if (c == -1) {
    Serial.println("Error: Invalid Cmd Blob");
    return;
  }
  cmd.CmdId = c;
    
  for (int i = 0; i < sizeof(cmd); ++i) {
    uint8_t c = (uint8_t)Wire.read();
    if (c == -1) {
      Serial.println("Error: Incomplete Cmd Blob");
      return;
    }
    cmd.u.AsRawBytes[i] = c;
  }
  
  switch (cmd.CmdId) {
    case MCU_CMD_SERVO_WRITE:
      Serial.print("MCU_CMD_SERVO_WRITE:");
      Serial.println(cmd.u.ServoWrite.Value);
      break;
      
    case MCU_CMD_ULTRASONIC_READ:
      Serial.println("MCU_CMD_ULTRASONIC_READ");
      break;
      
    default:
      Serial.println("Error: INVALID CMD");
      newCmdArrived = false;
      return;
  }
  
  newCmdArrived = true;
}

