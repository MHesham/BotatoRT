#include "precomp.h"
#include "AdafruitMotorHAT.h"
#include "..\BotatoMcu\BotatoMcu.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace BotatoRT;
using namespace Adafruit;
using namespace BotatoMcu;

void MotorsTest()
{
    auto motorDriver = ref new AdafruitMotorHAT(MaxPwmFrequency);

    motorDriver->Init();

    for (int i = 30; i <= 100; i += 10) {
        float power = (float)i / 100.0f;

        LogInfo("Using power %d", int(power * 100.0f));

        motorDriver->Forward(MotorID::Motor1, power);
        motorDriver->Forward(MotorID::Motor2, power);

        Sleep(3000);

        motorDriver->Coast(MotorID::Motor2);
        motorDriver->Coast(MotorID::Motor1);

        Sleep(1000);
    }
}

int main(Platform::Array<Platform::String^>^ args)
{
    LogInfo("Starting Botato, Hello World!");

    auto mcuDev = I2cHelper::MakeDevice(MCU_SLAVE_ADDR);
    LogInfo("Established MCU I2C Channel");

    Platform::Array<BYTE>^ cmdBlob = ref new Platform::Array<BYTE>(sizeof(McuCommand)) {};
    auto cmdPtr = (McuCommand*)cmdBlob->begin();
    cmdPtr->CmdId = MCU_CMD_SERVO_WRITE;
    cmdPtr->u.ServoWrite.ServoId = 0;

    for (uint8_t s = 0; s <= 180; s+=5) {
        cmdPtr->u.ServoWrite.Value = s;
        mcuDev->Write(cmdBlob);
        LogInfo("Sent ServoWrite:%d", s);
        Sleep(1000);
    }

    LogInfo("Botato exiting and going to sleep ZzZzZzzzz ...");

    return 0;
}
