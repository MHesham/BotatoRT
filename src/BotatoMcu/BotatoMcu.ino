#include <Wire.h>
#include <Servo.h>
#include <NewPing.h>
#include "..\Include\I2cMcu.h"
#include "LvMaxSonarEz.h"
#include "Sonar.h"

using namespace Botato::Mcu;

#define DEBUG_REG

enum Pin {
    PIN_SERVO0 = 8,
    PIN_SONAR0_TRIG = 10,
    PIN_SONAR0_ECHO = 9,
    PIN_MAXSONAR_PW,
    PIN_LEFT_ENCODER,
    PIN_RIGHT_ENCODER,
};

enum {
    SONAR_MAX_DISTANCE_CM = 100,
    SONAR_SAMPLE_INTERVAL_MS = 333,
    MAXSONAR_SAMPLE_INTERVAL_MS = 333,
};

class AutoLed
{
public:
    AutoLed()
    {
        digitalWrite(LED_BUILTIN, HIGH);
    }

    ~AutoLed()
    {
        digitalWrite(LED_BUILTIN, LOW);
    }
};

volatile McuRegister selectedRegAddr = MCU_REG_NOACCSS;
volatile McuWord_t registers[MCU_REG_COUNT];
Servo servo0;
NewPing sonar0(PIN_SONAR0_TRIG, PIN_SONAR0_ECHO, SONAR_MAX_DISTANCE_CM);
unsigned long sonarLastSampleTimeMs = 0;
Sonar<> sonar(PIN_SONAR0_TRIG, PIN_SONAR0_ECHO, SONAR_MAX_DISTANCE_CM);
LvMaxSonarEz<> maxSonar(PIN_MAXSONAR_PW, MAXSONAR_SAMPLE_INTERVAL_MS);
McuWord_t lastFps = 0;
McuWord_t numFrames = 0;
unsigned long lastFpsSampleTimeMs = 0;

volatile uint32_t registersDirtyBit;
#define IS_REG_DIRTY(REGADDR)           (registersDirtyBit & (1 << (REGADDR)))
#define REG_DIRTY_BIT_SET(REGADDR)      (registersDirtyBit |= (1 << (REGADDR)))
#define REG_DIRTY_BIT_UNSET(REGADDR)    (registersDirtyBit &= ~(1 << (REGADDR)))
static_assert(
    MCU_REG_COUNT <= 31,
    "Highest register address should not exceed 31");

void setup()
{
    // join i2c bus with specific address
    Wire.begin(MCU_I2C_SLAVE_ADDR);
    // set I2C speed to FastMode
    Wire.setClock(400000);

    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    memset((void*)registers, 0, sizeof(registers));

    servo0.attach(PIN_SERVO0);
    servo0.write(0);

    Serial.begin(115200);
    Serial.println("BotatoRT MCU Startup");

    registers[MCU_REG_IS_READY] = true;
}

void updateMotors()
{
    if (IS_REG_DIRTY(MCU_REG_SERVO_ANGLE_DEG)) {
        servo0.write(registers[MCU_REG_SERVO_ANGLE_DEG]);
        REG_DIRTY_BIT_UNSET(MCU_REG_SERVO_ANGLE_DEG);
    }
}

void updateSensors()
{
    if (millis() - sonarLastSampleTimeMs >= SONAR_SAMPLE_INTERVAL_MS) {
        registers[MCU_REG_SONAR_DISTANCE_CM] =
            sonar0.convert_cm(sonar0.ping_median());
        sonarLastSampleTimeMs = millis();
    }

    if (maxSonar.Update()) {
        registers[MCU_REG_MAXSONAR_DISTANCE_CM] = maxSonar.GetDistanceCm();
    }
}

void loop()
{
    unsigned long frameTimeMs = millis();
    unsigned long dt = frameTimeMs - lastFpsSampleTimeMs;
    if (dt > 1000u) {
        registers[MCU_REG_FPS] = (numFrames * 1000) / dt;
        lastFpsSampleTimeMs = frameTimeMs;
        numFrames = 0;
    }

    updateMotors();
    updateSensors();
    ++numFrames;
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{
    Wire.write((uint8_t*)registers[selectedRegAddr], sizeof(McuWord_t));
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int count)
{
    AutoLed led;

    // First byte should be reg addr
    int regAddr = Wire.read();
    if (regAddr < int(MCU_REG_BEGIN) ||
        regAddr >= int(MCU_REG_COUNT)) {
        Serial.println("Error: Invalid register address");
        return;
    }

    selectedRegAddr = McuRegister(regAddr);

    // In case this is a reg read, we will place the value bytes
    // in the requestEvent, otherwise this is a reg write
    if (!Wire.available())
        return;

    if (regAddr < MCU_REG_READONLY_END) {
        Serial.println("Error: Register is Read-only @");
        Serial.println(selectedRegAddr);
        return;
    }

    McuWord_t newVal;
    size_t byteCount = Wire.readBytes((uint8_t*)&newVal, sizeof(McuWord_t));
    if (byteCount != sizeof(McuWord_t)) {
        Serial.println("Error: Incomplete register value");
        return;
    }

    McuWord_t oldVal = registers[selectedRegAddr];

    // No value change, ignore it
    if (oldVal == newVal)
        return;

#ifdef DEBUG_REG
    switch (selectedRegAddr) {
    case MCU_REG_SERVO_ANGLE_DEG:
        Serial.print("Reg[MCU_REG_SERVO_ANGLE_DEG]=");
        Serial.println(newVal);
        break;
    }
#endif

    registers[selectedRegAddr] = newVal;
    REG_DIRTY_BIT_SET(selectedRegAddr);
}

