/**
 * @file single_sensor_wifi.cpp 
 * @author Jan Schmidt
 * @brief 
 * @version 0.1
 * @date 2025-06-23
 * 
 * @copyright Copyright (c) 2025
 * 
 * This example demonstrates how to use a single PAA5102E1 sensor with Zenoh over WiFi.
 * It initializes the sensor, sets up Zenoh for communication, and handles queryables for sensor
 * operations such as reading and writing sensor data.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <zenoh-pico.h>
#include <paa5102e1.hpp>
#include "secrets.h"
#include "sensor_queriables.hpp"

z_owned_publisher_t pub;
z_owned_session_t session;
z_publisher_put_options_t options;
std::unique_ptr<PAA5102E1> sensor_ptr;
std::unique_ptr<Z_PAA5102E1_Handler> handler_ptr;

z_owned_queryable_t q_reset, q_sleep, q_awake, q_isWriteProtected, q_isSleeping, q_isAwake,
    q_enableWriteProtection, q_disableWriteProtection, q_writeLaserDriveCurrent,
    q_readLaserDriveCurrent, q_readDeltaX, q_readDeltaY, q_readShutter,
    q_readFrameAvg, q_readImageQuality, q_readResolutionX, q_readResolutionY,
    q_writeResolutionX, q_writeResolutionY, q_start, q_stop, q_setFrequency;

void init_serial()
{
  Serial.begin(BAUD_RATE);
  while (!Serial)
  {
    delay(1000);
  }
  Serial.println("Serial initialized");
}

void init_wifi()
{
  Serial.print("Connecting to WiFi...\t");
  Serial.print("SSID: ");
  Serial.print(SSID_NAME);
  Serial.print(" PW: ");
  Serial.println(SSID_PW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_NAME, SSID_PW);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }
  Serial.println("OK");
}

void init_zenoh()
{
  // Initialize Zenoh Session and other parameters
  z_owned_config_t config;
  z_config_default(&config);
  zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, ZENOH_MODE);
  if (strcmp(LOCATOR, "") != 0)
  {
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, LOCATOR);
  }

  // Open Zenoh session
  Serial.print("Opening Zenoh Session... \t");
  if (z_open(&session, z_config_move(&config), NULL) < 0)
  {
    Serial.println("Unable to open session!");
    while (1)
    {
      ;
    }
  }
  Serial.println("OK");

  // Start read and lease tasks for zenoh-pico
  if (zp_start_read_task(z_session_loan_mut(&session), NULL) < 0 || zp_start_lease_task(z_session_loan_mut(&session), NULL) < 0)
  {
    Serial.println("Unable to start read and lease tasks\n");
    z_session_drop(z_session_move(&session));
    while (1)
    {
      ;
    }
  }
}

void setup()
{
  
  init_serial();
  init_wifi();
  init_zenoh();
  
  SPI.begin();
  sensor_ptr = std::make_unique<PAA5102E1>(SPISettings(1000000, MSBFIRST, SPI_MODE3), 5, 4);
  auto res = sensor_ptr->init();

  if (res.hasError)
  {
    Serial.printf("Failed to initialize sensor: %s\n", res.error.toString());
    while (1)
    {
      ;
    }
  }

  handler_ptr = std::make_unique<Z_PAA5102E1_Handler>(z_session_loan_mut(&session), "sensor/1", std::move(sensor_ptr));
  handler_ptr->setup_queryables();
}

void loop() {
  handler_ptr->loop();
}