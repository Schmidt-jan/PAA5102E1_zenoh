#pragma once

#include <zenoh-pico.h>
#include <paa5102e1.hpp>
#include <custom_msg/Delta.hpp>
#include <std_msg/Bool.hpp>
#include <std_msg/Byte.hpp>
#include <std_msg/UInt16.hpp>
#include <std_msg/UInt32.hpp>
#include <std_msg/Int32.hpp>
#include <std_msg/Empty.hpp>
#include <paa5102e1.hpp>
#include <memory>

#define DEFAULT_PUB_FREQ_HZ 10
#define DEFAULT_PUB_TOPIC "delta"

class Z_PAA5102E1_Handler
{
private:
    unsigned long last_send = 0;
    z_owned_publisher_t pub;
    const std::string name;
    const std::string keyexpr_publisher;
    const z_loaned_session_t *session;
    int64_t long_time_delta_x = 0;
    int64_t long_time_delta_y = 0;

    z_owned_queryable_t 
        qable_reset, 
        qable_sleep, qable_awake, qable_isSleeping, qable_isAwake,
        qable_isWriteProtected, qable_enableWriteProtection, qable_disableWriteProtection,
        qable_readLaserDriveCurrent, qable_writeLaserDriveCurrent, 
        qable_readShutter, qable_readFrameAvg, qable_readImageQuality, 
        qable_readResolutionX, qable_readResolutionY, qable_writeResolutionX, qable_writeResolutionY, 
        qable_readDeltaX, qable_readDeltaY, 
        qable_readAbsDeltaX, qable_readAbsDeltaY, qable_resetAbsDelta,
        qable_lightSourceLED, qable_lightSourceLaser,
        qable_publisherStart, qable_publisherStop, qable_publisherFrequency;

    void send_sensor_data();

    void declare_queryable(const std::string& keyexpr_suf, void (*callback)(z_loaned_query_t *, void *), z_owned_queryable_t &queryable);

public:
    std::shared_ptr<PAA5102E1> sensor;
    bool sending_enabled = false;
    uint32_t send_freq_hz = DEFAULT_PUB_FREQ_HZ;

    Z_PAA5102E1_Handler(
        z_loaned_session_t *session,
        const std::string& name,
        std::shared_ptr<PAA5102E1> sensor,
        std::string keyexpr_publisher)
        : sending_enabled(true),
          name(name),
          session(session),
          sensor(std::move(sensor)),
          send_freq_hz(DEFAULT_PUB_FREQ_HZ),
          long_time_delta_x(0),
          long_time_delta_y(0),
          keyexpr_publisher(keyexpr_publisher)
    {
        z_owned_config_t config;
        z_config_default(&config);
        zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, ZENOH_MODE);
        if (strcmp(LOCATOR, "") != 0)
        {
            zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, LOCATOR);
        }

        z_view_keyexpr_t ke;
        z_view_keyexpr_from_str_unchecked(&ke, keyexpr_publisher.c_str());
        if (z_declare_publisher(session, &pub, z_view_keyexpr_loan(&ke), NULL) < 0)
        {
            Serial.println("Unable to declare publisher for key expression!");
            while (1)
            {
                ;
            }
        }
    }

    Z_PAA5102E1_Handler(
        z_loaned_session_t *session,
        const std::string& name,
        std::shared_ptr<PAA5102E1> sensor)
    : Z_PAA5102E1_Handler(session, name, std::move(sensor), name + "/" + DEFAULT_PUB_TOPIC)
    {
    }

    void setup_queryables();

    void loop();
};
