#pragma once

namespace Botato {
    namespace Mcu {

        interface class ILowLevelMcu
        {
            bool IsReady();
            uint32_t GetFps();
            void ServoSetAngle(uint32_t degree);
            uint32_t SonarGetDistanceCm();
            uint32_t MaxSonarGetDistanceCm();

        };

        ref class McuI2cProxy sealed : ILowLevelMcu
        {
        public:
            McuI2cProxy()
            {
                using namespace Mcu;

                regWriteBuff_ = ref new Platform::Array<uint8_t>(3);
                regReadBuff_ = ref new Platform::Array<uint8_t>(2);
                regAddrReadBuff_ = ref new Platform::Array<uint8_t>(1);

                mcuDev_ = I2cHelper::MakeDevice(MCU_I2C_SLAVE_ADDR);
                LogInfo("Established MCU I2C channel at address 0x%x", MCU_I2C_SLAVE_ADDR);
            }

            virtual bool IsReady()
            {
                return readReg(MCU_REG_IS_READY) > 0;
            }

            virtual uint32_t GetFps()
            {
                return readReg(MCU_REG_FPS);
            }

            virtual void ServoSetAngle(uint32_t degree)
            {
                return writeReg(MCU_REG_SERVO_ANGLE_DEG, degree);
            }

            virtual uint32_t SonarGetDistanceCm()
            {
                return readReg(MCU_REG_SONAR_DISTANCE_CM);
            }

            virtual uint32_t MaxSonarGetDistanceCm()
            {
                return readReg(MCU_REG_MAXSONAR_DISTANCE_CM);
            }

        private:
            void writeReg(uint8_t regAddr, McuWord_t val)
            {
                regWriteBuff_[0] = regAddr;
                *((McuWord_t*)&regWriteBuff_[1]) = val;
                mcuDev_->Write(regWriteBuff_);
            }

            McuWord_t readReg(uint8_t regAddr)
            {
                regAddrReadBuff_[0] = regAddr;
                mcuDev_->WriteRead(regAddrReadBuff_, regReadBuff_);
                return *((McuWord_t*)regReadBuff_->begin());
            }

            Platform::Array<uint8_t>^ regWriteBuff_;
            Platform::Array<uint8_t>^ regReadBuff_;
            Platform::Array<uint8_t>^ regAddrReadBuff_;
            Windows::Devices::I2c::I2cDevice^ mcuDev_;
        };
    }
}