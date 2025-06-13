#include <Arduino.h>
#include <zenoh-pico.h>
#include <WiFi.h>
#include <supported_boards.hpp>

#define MODE "client"
#define LOCATOR ""
#define ZENOH_CONFIG_CONNECT "serial/UART_1#baudrate=115200"
#define ZENOH_CONFIG_LISTEN ""

z_owned_session_t s;
z_owned_subscriber_t sub;

z_owned_publisher_t pub;
static int idx = 0;

#define KEYEXPR "demo/example/zenoh-pico-pub"
#define VALUE "[ARDUINO]{ESP32} Publication from Zenoh-Pico!"

void setup() {
  delay(1000);
    //Serial.begin(115200);
    while (!Serial) { delay(1000);}
    Serial.println("Serial port initialized!");
    Serial1.begin(115200);
    while (!Serial1) { delay(1000);}
    // Serial.println("Serial1 port initialized!");
#ifndef Z_FEATURE_LINK_SERIAL
    Serial.print("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin("jan-hotspot", "4F501554446AC7E3BF7ECF21A4");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    Serial.println("OK");
#endif

    z_owned_config_t config;
    z_config_default(&config);
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, MODE);

#ifdef Z_FEATURE_LINK_SERIAL
    if (strcmp(ZENOH_CONFIG_CONNECT, "") != 0) {
      zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, ZENOH_CONFIG_CONNECT);
    }

    if (strcmp(ZENOH_CONFIG_LISTEN, "") != 0) {
      zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_LISTEN_KEY, ZENOH_CONFIG_LISTEN);
    }


    
    #else
    if (strcmp(LOCATOR, "") != 0) {
      if (strcmp(MODE, "client") == 0) {
        zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, LOCATOR);
      } else {
        zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_LISTEN_KEY, LOCATOR);
      }
    }
    #endif
    Serial.print("Opening Zenoh session... ");
    // Open Zenoh session
  z_result_t res = z_open(&s, z_config_move(&config), NULL);
  if (res < 0)
  {
    Serial.print("Unable to open session! Code: ");
    Serial.println(res);
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("OK");

  // Start read and lease tasks for zenoh-pico
  Serial.print("Starting read and lease tasks... ");
  /*
  if (zp_start_read_task(z_session_loan_mut(&s), NULL) < 0)
  {
    Serial.println("Unable to start read and lease tasks");
    z_session_drop(z_session_move(&s));
    while (1)
    {
      delay(1000);
    }
  }*/
    // Declare Zenoh publisher
    Serial.print("Declaring publisher for ");
    Serial.print(KEYEXPR);
    Serial.println("...");
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, KEYEXPR);
    if (z_declare_publisher(z_session_loan(&s), &pub, z_view_keyexpr_loan(&ke), NULL) < 0) {
        Serial.println("Unable to declare publisher for key expression!");
        while (1) {
            ;
        }
    }
    Serial.println("OK");
    Serial.println("Zenoh setup finished!");

    delay(300);
}

void loop() {
    delay(1000);
    char buf[256];
    sprintf(buf, "[%4d] %s", idx++, VALUE);

    Serial.print("Writing Data ('");
    Serial.print(KEYEXPR);
    Serial.print("': '");
    Serial.print(buf);
    Serial.println("')");

    // Create payload
    z_owned_bytes_t payload;
    z_bytes_copy_from_str(&payload, buf);

    if (z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error while publishing data");
    }
}