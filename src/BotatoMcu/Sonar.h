#ifndef __SONAR_H__
#define __SONAR_H__

namespace Botato {
    namespace Mcu {

        template<size_t SampleCount = 5>
        class Sonar
        {
        public:
            Sonar(uint8_t trigPin, uint8_t echoPin, unsigned int maxDistCm) :
                pinger_(trigPin, echoPin, maxDistCm)
            {}

            void Setup()
            {
            }

            void Update()
            {

            }

            uint32_t GetDistanceCm()
            {
            }

        private:

            void echoCheck()
            {
                if (pinger_.check_timer()) {
                    pinger_.ping_result / US_ROUNDTRIP_CM;
                }
            }

            unsigned long pwSamples_[SampleCount];
            NewPing pinger_;
        };
    }
}

#endif // __SONAR_H__