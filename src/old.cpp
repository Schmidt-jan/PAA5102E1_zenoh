/*#include <Arduino.h>
#include <WiFi.h>
#include <zenoh-pico.h>
#include <paa5102e1Array.hpp>
#include "secrets.h"
#include "sensor_array_queriables.hpp"

#define NUM_SENSORS 2

z_owned_publisher_t pub;
z_owned_session_t session;
z_publisher_put_options_t options;
std::unique_ptr<PAA5102E1> sensor_ptr1;
std::unique_ptr<Z_PAA5102E1_Handler> handler_ptr1;
std::unique_ptr<PAA5102E1> sensor_ptr2;
std::unique_ptr<Z_PAA5102E1_Handler> handler_ptr2;

std::unique_ptr<PAA5102E1Array<NUM_SENSORS>> sensor_ptr;
std::unique_ptr<Z_PAA5102E1_Array_Handler<NUM_SENSORS>> handler_ptr;

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
  zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_LISTEN_KEY, "serial/UART_1#baudrate=115200");
  // zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_LISTEN_KEY, "listen");

  // Open Zenoh session
  Serial.print("Opening Zenoh Session... \t");
  auto res = z_open(&session, z_config_move(&config), NULL);
  if (res < 0)
  {
    Serial.printf("Unable to open session! Error code: %d\n", res);
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

 const PAA5102E1PinSetting pinSettings[NUM_SENSORS] = {
    {GPIO_NUM_21, GPIO_NUM_22},
    {GPIO_NUM_4, GPIO_NUM_0},
  };

void setup()
{
  delay(1000);
  init_serial();
  //init_wifi();
  init_zenoh();
  
  SPI.begin();

  sensor_ptr = std::make_unique<PAA5102E1Array<NUM_SENSORS>>(SPISettings(1000000, MSBFIRST, SPI_MODE3), pinSettings);
  Serial.print("Initializing sensor array...\t");
  auto res = sensor_ptr->init(1);

  if (res.hasError)
  {
    Serial.printf("Failed to initialize sensor: %s\n", res.error.toString());
    while (1)
    {
      ;
    }
  }

  handler_ptr = std::make_unique<Z_PAA5102E1_Array_Handler<NUM_SENSORS>>(z_session_loan_mut(&session), "sensor", std::move(sensor_ptr));
  handler_ptr->setup_queryables();
}

void loop() {
  handler_ptr->loop();
}*/