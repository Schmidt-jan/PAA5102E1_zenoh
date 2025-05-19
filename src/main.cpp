#include <Arduino.h>
#include <WiFi.h>
#include <zenoh-pico.h>
#include "secrets.h"

#define PUB_FREQ_HZ 1
#define ZENOH_MODE "client"
#define LOCATOR ""

#define ROS_TOPIC "test"
#define KEYEXPR_PUB "0/" ROS_TOPIC "/std_msgs::msg::dds_::String_/RIHS01_df668c740482bbd48fb39d76a70dfd4bd59db1288021743503259e948f6b1a18"

z_owned_publisher_t pub;
z_owned_session_t s;
z_publisher_put_options_t options;

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
  if (z_open(&s, z_config_move(&config), NULL) < 0)
  {
    Serial.println("Unable to open session!");
    while (1)
    {
      ;
    }
  }
  Serial.println("OK");

  // Start read and lease tasks for zenoh-pico
  if (zp_start_read_task(z_session_loan_mut(&s), NULL) < 0 || zp_start_lease_task(z_session_loan_mut(&s), NULL) < 0)
  {
    Serial.println("Unable to start read and lease tasks\n");
    z_session_drop(z_session_move(&s));
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


  Serial.println("Declaring publisher for " + String(KEYEXPR_PUB));
  z_view_keyexpr_t ke_pub;

  z_view_keyexpr_from_str_unchecked(&ke_pub, KEYEXPR_PUB);
  if (z_declare_publisher(z_session_loan(&s), &pub, z_view_keyexpr_loan(&ke_pub), NULL) < 0)
  {
    Serial.println("Unable to declare publisher for key expression!");
    while (1)
    {
      ;
    }
  }
  

  // Init liveliness
  String idStr = "";
  for (int i = 0; i < ZENOH_ID_SIZE; i++) {
    if (s._rc._val->_local_zid.id[i] < 0x10) idStr += "0";  // Ensure 2-digit hex
    idStr += String(s._rc._val->_local_zid.id[i], HEX);
  }

  auto expr = String("@/" + idStr + "/@ros2_lv/MP/" + ROS_TOPIC + "/std_msgs::msg::String/:::");
  z_view_keyexpr_t liveliness_ke;
  z_view_keyexpr_from_str(&liveliness_ke, expr.c_str());

  z_loaned_session_t l_s = *z_session_loan(&s);
  z_owned_liveliness_token_t token;
  z_loaned_keyexpr_t keyexpr = *z_view_keyexpr_loan(&liveliness_ke);
  z_liveliness_token_options_t options;
  z_liveliness_token_options_default(&options);
  z_result_t res = z_liveliness_declare_token(&l_s, &token, &keyexpr, &options);
  if (res != _Z_RES_OK) {
    Serial.println("Error initializing liveliness token. Result code: " + String(res));
  } else {
    Serial.println("Liveliness sucessfully sent");
  }

  Serial.println("Initialization Done");
}

int64_t sequence_number = 0;


void loop()
{
  const uint8_t data[] = {
    0x00, 0x01, 0x00, 0x00,             // Encapsulation: dummy byte = 0x00, encoding = 0x01 (PLAIN_CDR, little endian), extend to four bytes
    0x06, 0x00, 0x00, 0x00,             // String length (6 bytes: "hello" + null)
    0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x00  // "hello\0"
};
  z_owned_bytes_t payload;
  z_bytes_from_static_buf(&payload, data, sizeof(data));

  z_publisher_put_options_default(&options);
  z_owned_bytes_t attachment;
  z_bytes_empty(&attachment);

  z_owned_encoding_t encoding;
  z_encoding_from_str(&encoding, "zenoh/string;utf8");
  z_moved_encoding_t *mvd_enc = z_encoding_move(&encoding);
  options.encoding = mvd_enc;

  z_timestamp_t ts;
  z_timestamp_new(&ts, z_session_loan(&s));
  options.timestamp = &ts;

  ze_owned_serializer_t serializer;
  ze_loaned_serializer_t *serializer_loan = ze_serializer_loan_mut(&serializer);

  ze_serializer_empty(&serializer);
  ze_serializer_serialize_str(serializer_loan, "sequence_number");
  ze_serializer_serialize_int64(serializer_loan, sequence_number++);
  ze_serializer_serialize_str(serializer_loan, "source_timestamp");
  ze_serializer_serialize_int64(serializer_loan, millis());
  ze_serializer_serialize_str(serializer_loan, "source_gid");
  ze_serializer_serialize_buf(serializer_loan, s._rc._val->_local_zid.id, ZENOH_ID_SIZE);
  ze_serializer_finish(ze_serializer_move(&serializer), &attachment);
  options.attachment = z_bytes_move(&attachment);

  Serial.print("Pub - ");
  z_result_t res = z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&payload), &options);
  if (res != _Z_RES_OK) {
    Serial.println("Error sending data. Result code: " + String(res));
  } else {
    Serial.println("Done");
  }
  delay(1000 / PUB_FREQ_HZ);
}