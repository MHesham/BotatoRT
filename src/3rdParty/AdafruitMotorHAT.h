// 
// Copyright 2015 Muhamad Lotfy
//
// The MIT License(MIT)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 

//
// Porting Adafruit-Motor-HAT-Python-Library to Windows 10 IoT Core C++/CX
// https://github.com/adafruit/Adafruit-Motor-HAT-Python-Library
//

#pragma once

#define Log(S, ...) wprintf_s(L##S##"\n", __VA_ARGS__)

namespace Adafruit {

    class wexception
    {
    public:
        explicit wexception(const std::wstring &msg) : msg_(msg) {}
        virtual ~wexception() { /*empty*/ }

        virtual const wchar_t *wwhat() const
        {
            return msg_.c_str();
        }

    private:
        std::wstring msg_;
    };

    class I2cHelper
    {
    public:

        static Windows::Devices::I2c::I2cDevice^ MakeDevice(int slaveAddress)
        {
            using namespace Windows::Devices::Enumeration;
            using namespace Windows::Devices::I2c;

            auto aqs = I2cDevice::GetDeviceSelector();
            auto dis = concurrency::create_task(DeviceInformation::FindAllAsync(aqs)).get();
            if (dis->Size == 0) {
                throw wexception(L"I2C bus not found");
            }

            auto id = dis->GetAt(0)->Id;
            auto settings = ref new I2cConnectionSettings(slaveAddress);
            settings->BusSpeed = I2cBusSpeed::FastMode;
            auto device = concurrency::create_task(I2cDevice::FromIdAsync(
                id,
                ref new I2cConnectionSettings(slaveAddress))).get();

            if (!device) {
                std::wostringstream msg;
                msg << L"Slave address 0x" << std::hex << slaveAddress << L" on bus " << id->Data()
                    << L" is in use. Please ensure that no other applications are using I2C.";
                throw wexception(msg.str());
            }

            return device;
        }

        static void GeneralCall(BYTE cmd)
        {
            auto generalCallDev = I2cHelper::MakeDevice(GeneralCallSlaveAddress);

            Log("Sending 0x%x general call to all devices on the bus", cmd);

            generalCallDev->Write(ref new Platform::Array<BYTE>(1) { cmd });
        }

    private:

        static const int GeneralCallSlaveAddress = 0;
    };

    enum PCA9685PwmFrequency
    {
        MinPmwFrequency = 24,
        MaxPwmFrequency = 1526
    };

    ref class PCA9685 sealed
    {
    public:

        PCA9685(int slaveAddress)
        {
            slaveAddress_ = slaveAddress;

            regWriteBuff_ = ref new Platform::Array<BYTE>(2);
            regReadBuff_ = ref new Platform::Array<BYTE>(1);
        }

        void Init()
        {
            I2cHelper::GeneralCall(SWRST);
            Sleep(5);

            Log("Initializing PCA9685");

            dev_ = I2cHelper::MakeDevice(slaveAddress_);

            SetAllChannelsDutyCycle(0.0f);
            // Output drive mode is totem-pole not open drain
            writeReg(MODE2, OUTDRV);
            // Turn-off oscillator and acknowledge All-Call transfers
            writeReg(MODE1, ALLCALL);
            Sleep(5);

            BYTE mode1 = readReg(MODE1);
            // Trun-on oscillator
            mode1 &= ~SLEEP;
            writeReg(MODE1, mode1);
            Sleep(5);
        }

        void SetPwmFrequency(int freq)
        {
            float prescaleval = 25000000.0f; // 25MHz
            prescaleval /= 4096.0f; // 12-bit
            prescaleval /= (float)freq;
            prescaleval -= 1.0f;

            Log(
                "Setting PWM frequency to %dHz, Estimated pre-scale: %f",
                freq,
                prescaleval);

            float prescale = (float)floor((double)prescaleval + 0.5);
            int effectiveFreq = (int)(25000000.0f / ((prescale + 1.0f) * 4096.0f));
            Log("Final pre-scale: %f with effective frequency %dHz", prescale, effectiveFreq);

            BYTE oldmode = readReg(MODE1);
            // Sleep to turn-off oscillator and disable Restart
            BYTE newmode = (BYTE)((oldmode & 0x7F) | 0x10);
            writeReg(MODE1, newmode);

            writeReg(PRESCALE, (BYTE)prescale);

            // Wake-up and enable oscillator
            writeReg(MODE1, oldmode);
            // Enable Restart
            writeReg(MODE1, (BYTE)(oldmode | 0x80));
        }

        void SetChannelDutyCycle(int channel, float dutyCycle)
        {
            if (dutyCycle < 0.0f)
                dutyCycle = 0.0f;
            else if (dutyCycle > 1.0f)
                dutyCycle = 1.0f;

            if (channel < 0 || channel > 15) {
                throw wexception(L"PWM channel must be in the range [0,15]");
            }

            int onTime = (int)(dutyCycle * (float)PwmCounterMax);
            _ASSERTE(
                (onTime >= 0 && onTime <= PwmCounterMax) &&
                "Channel signal on time must be in range [0,4095]");

            BYTE offset = (BYTE)(4 * channel);
            writeReg((BYTE)(LED0_ON_L + offset), 0);
            writeReg((BYTE)(LED0_ON_H + offset), 0);
            writeReg((BYTE)(LED0_OFF_L + offset), (BYTE)(onTime & 0xFF));
            writeReg((BYTE)(LED0_OFF_H + offset), (BYTE)(onTime >> 8));

            Log(
                "Channel#%d Duty=%f ON=%d",
                channel,
                dutyCycle,
                onTime);
        }

        void SetAllChannelsDutyCycle(float dutyCycle)
        {
            if (dutyCycle < 0.0f)
                dutyCycle = 0.0f;
            else if (dutyCycle > 1.0f)
                dutyCycle = 1.0f;

            int onTime = (int)(dutyCycle * (float)PwmCounterMax);
            _ASSERTE(
                (onTime >= 0 && onTime <= PwmCounterMax) &&
                "Channel signal on time must be in range [0,4095]");

            writeReg(ALL_LED_ON_L, 0);
            writeReg(ALL_LED_ON_H, 0);
            writeReg(ALL_LED_OFF_L, (BYTE)(onTime & 0xFF));
            writeReg(ALL_LED_OFF_H, (BYTE)(onTime >> 8));

            Log("All Channel Duty=%f ON=%d", dutyCycle, onTime);
        }

    private:

        static const BYTE MODE1 = 0x00;
        static const BYTE MODE2 = 0x01;
        static const BYTE SUBADDR1 = 0x02;
        static const BYTE SUBADDR2 = 0x03;
        static const BYTE PRESCALE = 0xFE;
        static const BYTE LED0_ON_L = 0x06;
        static const BYTE LED0_ON_H = 0x07;
        static const BYTE LED0_OFF_L = 0x08;
        static const BYTE LED0_OFF_H = 0x09;
        static const BYTE ALL_LED_ON_L = 0xFA;
        static const BYTE ALL_LED_ON_H = 0xFB;
        static const BYTE ALL_LED_OFF_L = 0xFC;
        static const BYTE ALL_LED_OFF_H = 0xFD;
        static const BYTE RESTART = 0x80;
        static const BYTE SLEEP = 0x10;
        static const BYTE ALLCALL = 0x01;
        static const BYTE INVRT = 0x10;
        static const BYTE OUTDRV = 0x04;
        static const BYTE SWRST = 0x6;

        static const int PwmCounterMax = 4095;

        void writeReg(BYTE regAddr, BYTE val)
        {
            regWriteBuff_[0] = regAddr;
            regWriteBuff_[1] = val;
            dev_->Write(regWriteBuff_);
        }

        BYTE readReg(BYTE regAddr)
        {
            regReadBuff_[0] = regAddr;
            dev_->Read(regReadBuff_);
            return regReadBuff_[0];
        }

        int slaveAddress_;
        Platform::Array<BYTE>^ regWriteBuff_;
        Platform::Array<BYTE>^ regReadBuff_;
        Windows::Devices::I2c::I2cDevice^ dev_;
    };

    public enum class MotorID
    {
        Motor1,
        Motor2,
        Motor3,
        Motor4
    };

    public enum class ExternalPwmChannel
    {
        Channel0 = 0,
        Channel1 = 1,
        Channe14 = 14,
        Channe15 = 15
    };

    ref class AdafruitMotorHAT sealed
    {
    public:

        AdafruitMotorHAT(int pwmFreq)
        {
            pwmFreq_ = pwmFreq;
            pwmDriver_ = ref new PCA9685(PCA9685SlaveAddress);
        }

        void Init()
        {
            pwmDriver_->Init();
            pwmDriver_->SetPwmFrequency(pwmFreq_);
            pwmDriver_->SetAllChannelsDutyCycle(0.0f);
        }

        void Forward(MotorID motor, float power)
        {
            Log("Motor%d Forward %d", motor, int(power * 100.0f));

            auto& motorConfig = Config[(int)motor];
            writePin(motorConfig.IN2, false);
            writePin(motorConfig.IN1, true);
            pwmDriver_->SetChannelDutyCycle(motorConfig.PwmChannel, power);
        }

        void Backward(MotorID motor, float power)
        {
            Log("Motor%d Backward %d", motor, int(power * 100.0f));

            auto& motorConfig = Config[(int)motor];
            writePin(motorConfig.IN1, false);
            writePin(motorConfig.IN2, true);
            pwmDriver_->SetChannelDutyCycle(motorConfig.PwmChannel, power);
        }

        void Coast(MotorID motor)
        {
            Log("Motor%d Coast", motor);

            auto& motorConfig = Config[(int)motor];
            writePin(motorConfig.IN1, false);
            writePin(motorConfig.IN2, false);
        }

        void SetExternalPwmChannelDutyCycle(ExternalPwmChannel channel, float dutyCycle)
        {
            pwmDriver_->SetChannelDutyCycle((int)channel, dutyCycle);
        }

    private:

        static const int PCA9685SlaveAddress = 0x60;

        struct MotorConfig
        {
            int PwmChannel;
            int IN1;
            int IN2;
        } Config[4] = {
            // M1
            { 8, 10, 9 },
            // M2
            { 13, 11, 12 },
            // M3
            { 2, 4, 3 },
            // M4
            { 7, 5, 6 }
        };

        void writePin(int pin, bool value)
        {
            if (value)
                pwmDriver_->SetChannelDutyCycle(pin, 1.0f);
            else
                pwmDriver_->SetChannelDutyCycle(pin, 0.0f);
        }

        PCA9685^ pwmDriver_;
        int pwmFreq_;
    };
} // namespace Adafruit