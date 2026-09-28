#include <Arduino.h>
#include <zephyr/fs/fs.h>
#include <ArduinoJson.h>

#define JSON_FILENAME "/storage/security.jsn"

void setup() {
  char wifi_ssid[256];
  char wifi_passwd[256];
  char location_city[256];
  char location_sample_time[10];
  Serial.begin(115200);
  while (!Serial) {}

  get_string("Enter Wifi SSID:", wifi_ssid);
  get_string("Password:", wifi_passwd);
  get_string("Weather city or zip:", location_city);
  get_string("Weather time between samples in minutes:", location_sample_time);

  JsonDocument doc;

  JsonObject wifi = doc["wifi"].to<JsonObject>();
  wifi["ssid"] = wifi_ssid;
  wifi["password"] = wifi_passwd;

  JsonObject weather_loc = doc["weather"].to<JsonObject>();
  weather_loc["city"] = location_city;
  float sample_time = atof(location_sample_time);
  weather_loc["cycle_time"] = sample_time;

  char buffer[512];  // ensure it's large enough
  size_t len = serializeJson(doc, buffer, sizeof(buffer));
  /* Write to file (example: MemFS) */
  buffer[len] = '\0';
  Serial.print("Generated Json: ");
  Serial.println(buffer);

  Serial.println("Trying to write to ");
  Serial.println(JSON_FILENAME);
  int ret = write_json_file(buffer, len);
  Serial.print("Return code: ");
  Serial.println(ret);

  // Now see if we can read in the JSON file
  Serial.println("Trying to read Json file");
  ret = read_json_file(buffer, len);
  Serial.print("Return code: ");
  Serial.println(ret);

}

int write_json_file(const char *buffer, size_t len) {
  struct fs_file_t file;
  fs_file_t_init(&file);
  int ret;
  ret = fs_open(&file, JSON_FILENAME, FS_O_CREATE | FS_O_WRITE);
  if (ret < 0) {
    Serial.println("failed to create file");
    return ret;
  }

  ret = fs_write(&file, buffer, len);
  if (ret < 0) {
    Serial.println("Failed to write to file");
    return ret;
  }

  ret = fs_close(&file);
  if (ret < 0) {
    Serial.println("Failed to close file");
    return ret;
  }
  Serial.println("File written successfully\n");
  return 0;
}

int read_json_file(const char *buffer, size_t len) {
  struct fs_file_t file;
  fs_file_t_init(&file);
  int ret;
  ret = fs_open(&file, JSON_FILENAME, FS_O_READ);
  if (ret < 0) {
    Serial.println("failed to open file");
    return ret;
  }


  ssize_t cb_read = fs_read(&file, (void*)buffer, len);
  if (cb_read < 0) {
    Serial.println("Failed to write to file");
    return cb_read;
  }

  ret = fs_close(&file);
  if (ret < 0) {
    Serial.println("Failed to close file");
    return ret;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, buffer, cb_read);
  if (error) {
    Serial.print("Failed to deserialize: ");
    Serial.println(error.c_str());
  }

  serializeJsonPretty(doc, Serial);
  Serial.println();
  return 0;
}



void get_string(const char *title, char *sz) {
  while (Serial.read() != -1) {}
  int chr;
  Serial.println(title);
  while ((chr = Serial.read()) == -1) {}
  while (chr >= ' ') {
    *sz++ = chr;
    chr = Serial.read();
  }
  *sz = '\0';
}


void loop() {
}
