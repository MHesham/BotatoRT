#ifndef __LVMAXSONAREZ_H__
#define __LVMAXSONAREZ_H__

#define INCH_TO_CM 2.54

namespace Botato {
    namespace Mcu {

        template<size_t SampleCount = 5>
        class LvMaxSonarEz
        {
        public:
            enum {
                MAXSONAR_SAMPLE_COUNT = 5,
                MAXSONAR_US_TO_INCH_SCALE = 147,
                // Pulse width > 0.88ms and < 37.5ms
                MAXSONAR_MAX_PULSE_WIDTH_US = 37500
            };

            LvMaxSonarEz(uint8_t pwPin, unsigned long samplingIntervalMs) :
                pwPin_(pwPin),
                lastSampleTimeMs_(0),
                samplingIntervalMs_(samplingIntervalMs)
            {}

            void Setup()
            {
                pinMode(pwPin_, INPUT);
                // Power-up time by datasheet
                delay(250);
                auto pwUs = readPulseWidth();
                for (size_t i = 0; i < SampleCount; ++i)
                    pwSamples_[i] = pwUs;
            }

            // return true if sampling occurred, false otherwise
            bool Update()
            {
                if (millis() - lastSampleTimeMs_ < samplingIntervalMs_)
                    return false;

                lastSampleTimeMs_ = millis();

                pwSamples_[0] = readPulseWidth();
                for (size_t i = 1; i < SampleCount; ++i) {
                    unsigned long x = readPulseWidth();
                    size_t j = i;
                    while (j > 0 && pwSamples_[j - 1] > x) {
                        pwSamples_[j] = pwSamples_[j - 1];
                        --j;
                    }
                    pwSamples_[j] = x;
                }

                return true;
            }

            uint16_t GetDistanceCm()
            {
                // return median reading
                auto medianDistanceInch =
                    pwSamples_[SampleCount / 2] / MAXSONAR_US_TO_INCH_SCALE;
                return medianDistanceInch * float(INCH_TO_CM);
            }

        private:
            unsigned long readPulseWidth()
            {
                return pulseIn(pwPin_, HIGH, MAXSONAR_MAX_PULSE_WIDTH_US);
            }

            uint8_t pwPin_;
            unsigned long pwSamples_[SampleCount];
            unsigned long lastSampleTimeMs_;
            const unsigned long samplingIntervalMs_;
        };
    }

#endif // __LVMAXSONAREZ_H__