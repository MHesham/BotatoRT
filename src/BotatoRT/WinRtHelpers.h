#pragma once

namespace Botato {
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

      LogInfo("Sending 0x%x general call to all devices on the bus", cmd);

      generalCallDev->Write(ref new Platform::Array<BYTE>(1) { cmd });
    }

  private:

    static const int GeneralCallSlaveAddress = 0;
  };
}