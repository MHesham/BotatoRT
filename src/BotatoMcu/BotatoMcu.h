#ifndef __BOTATO_MCU_H__
#define __BOTATO_MCU_H__

#include <inttypes.h>

namespace BotatoMcu {

enum {
  MCU_SLAVE_ADDR = 8
};

enum : uint8_t {
  MCU_CMD_INVALID,
  MCU_CMD_SERVO_WRITE,
  MCU_CMD_ULTRASONIC_READ
};

#pragma pack(push,1)
struct McuCommand {
    McuCommand(uint8_t cmdId = MCU_CMD_INVALID) :
        CmdId(cmdId)
    {}

    uint8_t CmdId;
    union {
        struct {
            uint8_t ServoId;
            uint8_t Value;
        } ServoWrite;

        struct {
            uint8_t SensorId;
        } UltrasonicRead;

        uint8_t AsRawBytes[3];
    } u;
};
#pragma pack(pop) // pack(push,1)
}

#endif // __BOTATO_MCU_H__
