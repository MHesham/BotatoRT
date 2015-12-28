#include "precomp.h"
#include "I2cMcu.h"
#include "LowLevelMcu.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Botato::Mcu;

int main(Platform::Array<Platform::String^>^ args)
{
    LogInfo("Starting Botato, Hello World!");

    ILowLevelMcu^ ctrlr = ref new McuI2cProxy();
    while (!ctrlr->IsReady());

    ctrlr->ServoSetAngle(90);

    for (int i = 0; i < 10; ++i) {
        LogInfo(
            "FPS=%d, SonarDistance=%dcm, MaxSonarDistance=%dcm",
            ctrlr->GetFps(),
            ctrlr->SonarGetDistanceCm(),
            ctrlr->MaxSonarGetDistanceCm());
        Sleep(2000);
    }

    LogInfo("Botato exiting and going to sleep ZzZzZzzzz ...");

    return 0;
}
