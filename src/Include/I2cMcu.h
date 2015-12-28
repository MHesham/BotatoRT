#ifndef __MCU_H__
#define __MCU_H__

#include <inttypes.h>

namespace Botato {
    namespace Mcu {

        enum {
            MCU_I2C_SLAVE_ADDR = 0x8
        };

        enum McuRegister : uint8_t {
            MCU_REG_NOACCSS,
            MCU_REG_IS_READY,
            MCU_REG_READONLY_START = MCU_REG_IS_READY,
            MCU_REG_BEGIN = MCU_REG_IS_READY,
            MCU_REG_FPS,
            MCU_REG_SONAR_DISTANCE_CM,
            MCU_REG_MAXSONAR_DISTANCE_CM,
            MCU_REG_LEFT_RIGHT_MOTOR_RPM,
            MCU_REG_SERVO_ANGLE_DEG,
            MCU_REG_READONLY_END = MCU_REG_SERVO_ANGLE_DEG,
            MCU_REG_COUNT = MCU_REG_SERVO_ANGLE_DEG
        };

        typedef uint32_t McuWord_t;
    }
}

#endif // __MCU_H__
