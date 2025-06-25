#pragma once

#include "sensor_queriables.hpp"
#include <paa5102e1Array.hpp>
#include <array>

#define DEFAULT_PUB_FREQ_HZ 10
#define DEFAULT_PUB_TOPIC "delta"

template <std::size_t N>
class Z_PAA5102E1_Array_Handler
{
private:
    std::array<std::unique_ptr<Z_PAA5102E1_Handler>, N> sensors;

public:
    Z_PAA5102E1_Array_Handler(
        z_loaned_session_t *session,
        const std::string& name,
        std::unique_ptr<PAA5102E1Array<N>> sensors_ptr,
        std::string keyexpr_publisher)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            Result<std::shared_ptr<PAA5102E1>> res = sensors_ptr->getSensor(i);
            if (res.hasError)
            {
                Serial.printf("Error getting sensor %zu: %s\n", i, res.error.getMessage().c_str());
                continue;
            }
            this->sensors[i] = std::make_unique<Z_PAA5102E1_Handler>(
                session,
                name + "/" + std::to_string(i),
                res.value,
                name + "/" + std::to_string(i) + "/" +  keyexpr_publisher);
        }
    }

    Z_PAA5102E1_Array_Handler(
    z_loaned_session_t *session,
    const std::string& name,
    std::unique_ptr<PAA5102E1Array<N>> sensors_ptr)
    : Z_PAA5102E1_Array_Handler(session, name, std::move(sensors_ptr), DEFAULT_PUB_TOPIC)
    {}
    
    void setup_queryables()
    {
        for (auto& sensor : sensors)
        {
            sensor->setup_queryables();
        }
    }

    void loop()
    {
        for (auto& sensor : sensors)
        {
            sensor->loop();
        }
    }  
};
