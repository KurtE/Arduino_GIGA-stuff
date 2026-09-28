
#include <WiFi.h>
#include <ArduinoJson.h>
#include <stdio.h>

#include "arduino_secrets.h"
#ifndef DEFAULT_CITY
#define DEFAULT_CITY "Los+Angeles"
#endif

//#define LARGE_HOURLY_DATA

String weather_city = DEFAULT_CITY;
String weather_time_zone = "";
double weather_latitude = 0;
double weather_longitude = 0;


//using namespace qindesign::network;

const char *server = "api.open-meteo.com";
const char *geocoding_api_server = "geocoding-api.open-meteo.com";

const int port = 80;
constexpr uint32_t kDHCPTimeout = 15000;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;           // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
// IPAddress server(93,184,216,34);  // IP address for example.com (no DNS)


ZephyrClient client;

// Allocated document capacity for hourly data arrays
JsonDocument doc;

// Global state tracking
enum AppState {
  START,
  FETCH_MAP_CITY_TO_LOCATION,
  READ_MAP_CITY_TO_LOCATION,
  FETCH_CURRENT,
  READ_CURRENT,
  FETCH_HOURLY,
  READ_HOURLY,
  FETCH_DAILY,
  READ_DAILY,
  DONE
};

AppState appState = START;

// Header detection state machine across loop iterations
uint8_t headerState = 0;
bool headerComplete = false;


void Serial_printf(const char *format, ...) {
  char buffer[256];
  va_list ap;
  va_start(ap, format);
  int cb_ret = vsnprintf(buffer, sizeof(buffer), format, ap);
  Serial.write(buffer, cb_ret);
}

// Helper to construct exact URL query
String buildQueryString(const char *basePath, JsonDocument &params) {
  String query = String(basePath);
  bool first = true;

  JsonObject obj = params.as<JsonObject>();
  for (JsonPair kv : obj) {
    query += first ? '?' : '&';
    first = false;

    query += kv.key().c_str();
    query += '=';

    if (kv.value().is<JsonArray>()) {
      bool firstElement = true;
      for (JsonVariant val : kv.value().as<JsonArray>()) {
        if (!firstElement) query += ',';
        query += val.as<String>();
        firstElement = false;
      }
    } else {
      query += kv.value().as<String>();
    }
  }
  return query;
}

// Convert Open-Meteo weather codes to human-readable strings
const char *getWeatherDescription(int code) {
  switch (code) {
    case 0: return "Clear sky";
    case 1: return "Mainly clear";
    case 2: return "Partly cloudy";
    case 3: return "Overcast";
    case 45: return "Fog";
    case 48: return "Depositing rime fog";
    case 51: return "Light drizzle";
    case 53: return "Moderate drizzle";
    case 55: return "Dense drizzle";
    case 61: return "Slight rain";
    case 63: return "Moderate rain";
    case 65: return "Heavy rain";
    case 71: return "Slight snow";
    case 73: return "Moderate snow";
    case 75: return "Heavy snow";
    case 80: return "Slight rain showers";
    case 81: return "Moderate rain showers";
    case 82: return "Violent rain showers";
    case 95: return "Thunderstorm";
    default: return "Unknown";
  }
}

// -------------------------------------------------------------------
// Request Builders
// -------------------------------------------------------------------
bool sendMapCityRequest() {
  client.stop();  // Clear existing socket
  if (!client.connect(geocoding_api_server, port)) return false;

  JsonDocument params;
  // make sure that city is in a format that is allowed
  weather_city.replace(' ', '+');
  params["name"] = weather_city;
  params["count"] = 1;

  String resource = buildQueryString("/v1/search", params);
  Serial.print("Name Query:");
  Serial.println(resource);

  client.print("GET ");
  client.print(resource.c_str());
  client.print(" HTTP/1.0\r\nHost: geocoding-api.open-meteo.com\r\nConnection: close\r\n\r\n");
  headerState = 0;
  headerComplete = false;
  return true;
}

bool sendCurrentRequest() {
  client.stop();  // Clear existing socket
  if (!client.connect(server, port)) return false;

  JsonDocument params;
  params["latitude"] = weather_latitude;
  params["longitude"] = weather_longitude;

  JsonArray current = params["current"].to<JsonArray>();
  current.add("temperature_2m");
  current.add("wind_speed_10m");
  current.add("wind_direction_10m");
  current.add("weather_code");
  current.add("pressure_msl");
  current.add("rain");
  current.add("snowfall");

  params["timezone"] = weather_time_zone;
  params["wind_speed_unit"] = "mph";
  params["temperature_unit"] = "fahrenheit";
  params["precipitation_unit"] = "inch";

  String resource = buildQueryString("/v1/forecast", params);

  client.print("GET ");
  client.print(resource.c_str());
  client.print(" HTTP/1.0\r\nHost: api.open-meteo.com\r\nConnection: close\r\n\r\n");

  headerState = 0;
  headerComplete = false;
  return true;
}

bool sendHourlyRequest() {
  client.stop();  // Clear existing socket
  if (!client.connect(server, port)) return false;

  JsonDocument params;
  params["latitude"] = weather_latitude;
  params["longitude"] = weather_longitude;

  JsonArray hourly = params["hourly"].to<JsonArray>();
  hourly.add("temperature_2m");
  hourly.add("precipitation_probability");
  hourly.add("precipitation");
#ifdef LARGE_HOURLY_DATA
  hourly.add("rain");
  hourly.add("snowfall");
  hourly.add("pressure_msl");
#endif  
  hourly.add("wind_speed_10m");

  params["timezone"] = weather_time_zone;
  params["forecast_days"] = 1;
  params["wind_speed_unit"] = "mph";
  params["temperature_unit"] = "fahrenheit";
  params["precipitation_unit"] = "inch";

  String resource = buildQueryString("/v1/forecast", params);

  client.print("GET ");
  client.print(resource.c_str());
  client.print(" HTTP/1.0\r\nHost: api.open-meteo.com\r\nConnection: close\r\n\r\n");

  headerState = 0;
  headerComplete = false;
  return true;
}

bool sendDailyRequest() {
  client.stop();  // Clear existing socket
  if (!client.connect(server, port)) return false;

  JsonDocument params;
  params["latitude"] = weather_latitude;
  params["longitude"] = weather_longitude;

  JsonArray daily = params["daily"].to<JsonArray>();
  daily.add("temperature_2m_max");
  daily.add("temperature_2m_min");
  daily.add("snowfall_sum");
  daily.add("precipitation_probability_max");
  daily.add("weather_code");
  daily.add("wind_speed_10m_max");
  daily.add("wind_gusts_10m_max");
  daily.add("rain_sum");
  daily.add("precipitation_sum");

  params["timezone"] = "America/New_York";
  params["wind_speed_unit"] = "mph";
  params["temperature_unit"] = "fahrenheit";
  params["precipitation_unit"] = "inch";

  String resource = buildQueryString("/v1/forecast", params);

  client.print("GET ");
  client.print(resource.c_str());
  client.print(" HTTP/1.0\r\nHost: api.open-meteo.com\r\nConnection: close\r\n\r\n");

  headerState = 0;
  headerComplete = false;
  return true;
}

// -------------------------------------------------------------------
// Render Output Functions
// -------------------------------------------------------------------
void printMapCityData() {
  Serial.println("\n====================== Map City ======================");

  // Lets remember this data for retrieving the weather data.
  const char *name = doc["results"][0]["name"];
  weather_latitude = doc["results"][0]["latitude"];
  weather_longitude = doc["results"][0]["longitude"];
  weather_time_zone = String(doc["results"][0]["timezone"]);

  Serial_printf("Name:                %s\n", name);
  Serial_printf("latitude:            %f\n", weather_latitude);
  Serial_printf("longitude:           %f\n", weather_longitude);
  Serial_printf("Time Zone:           %s\n", weather_time_zone.c_str());
}

void printCurrentData() {
  Serial.println("\n====================== CURRENT WEATHER ======================");
  Serial_printf("Time:                %s\n", doc["current"]["time"].as<const char *>());
  Serial_printf("Condition:           %s (Code %d)\n",
                getWeatherDescription(doc["current"]["weather_code"].as<int>()),
                doc["current"]["weather_code"].as<int>());
  Serial_printf("Temperature:         %.1f °F\n", doc["current"]["temperature_2m"].as<float>());
  Serial_printf("Pressure (MSL):      %.1f hPa\n", doc["current"]["pressure_msl"].as<float>());
  Serial_printf("Wind Speed:          %.1f mph\n", doc["current"]["wind_speed_10m"].as<float>());
  Serial_printf("Wind Direction:      %d°\n", doc["current"]["wind_direction_10m"].as<int>());
  Serial_printf("Rain:                %.2f in\n", doc["current"]["rain"].as<float>());
  Serial_printf("Snowfall:            %.2f in\n", doc["current"]["snowfall"].as<float>());
}

void printHourlyData() {
  JsonArray hourlyTime = doc["hourly"]["time"].as<JsonArray>();
  JsonArray hourlyTemp = doc["hourly"]["temperature_2m"].as<JsonArray>();
  JsonArray hourlyProb = doc["hourly"]["precipitation_probability"].as<JsonArray>();
  JsonArray hourlyPrecip = doc["hourly"]["precipitation"].as<JsonArray>();
#ifdef LARGE_HOURLY_DATA
  JsonArray hourlyRain = doc["hourly"]["rain"].as<JsonArray>();
  JsonArray hourlySnow = doc["hourly"]["snowfall"].as<JsonArray>();
  JsonArray hourlyPress = doc["hourly"]["pressure_msl"].as<JsonArray>();
#endif

  JsonArray hourlyWind = doc["hourly"]["wind_speed_10m"].as<JsonArray>();

  if (hourlyTime.size() > 0) {
    Serial.println("\n========================================== HOURLY FORECAST ==========================================");
#ifndef LARGE_HOURLY_DATA
    Serial.println("   Time       Temp     PoP%    Precipitation     Wind");
#else
    Serial.println("   Time       Temp     PoP%    Precipitation     Rain          Snow       Pressure     Wind");
#endif
    Serial.println("-----------------------------------------------------------------------------------------------------");

    size_t hoursToPrint = min(hourlyTime.size(), (size_t)24);
    for (size_t i = 0; i < hoursToPrint; i++) {
      const char *fullTime = hourlyTime[i];
      const char *timeOnly = (strlen(fullTime) >= 16) ? (fullTime + 11) : fullTime;

#ifndef LARGE_HOURLY_DATA
      Serial_printf("   %-8s  %4.1f °F  %3d%%       %5.2f in      %4.1f mph\n",
                    timeOnly,
                    hourlyTemp[i].as<float>(),
                    hourlyProb[i].as<int>(),
                    hourlyPrecip[i].as<float>(),
                    hourlyWind[i].as<float>());
#else
      Serial_printf("   %-8s  %4.1f °F  %3d%%       %5.2f in      %5.2f in      %5.2f in      %4.1f    %4.1f mph\n",
                    timeOnly,
                    hourlyTemp[i].as<float>(),
                    hourlyProb[i].as<int>(),
                    hourlyPrecip[i].as<float>(),
                    hourlyRain[i].as<float>(),
                    hourlySnow[i].as<float>(),
                    hourlyPress[i].as<float>(),
                    hourlyWind[i].as<float>());
#endif
    }
  }
}

void printDailyData() {
  JsonArray dailyTime = doc["daily"]["time"].as<JsonArray>();
  JsonArray dailyMax = doc["daily"]["temperature_2m_max"].as<JsonArray>();
  JsonArray dailyMin = doc["daily"]["temperature_2m_min"].as<JsonArray>();
  JsonArray dailyPop = doc["daily"]["precipitation_probability_max"].as<JsonArray>();
  JsonArray dailyPrecip = doc["daily"]["precipitation_sum"].as<JsonArray>();
  JsonArray dailyRain = doc["daily"]["rain_sum"].as<JsonArray>();
  JsonArray dailySnow = doc["daily"]["snowfall_sum"].as<JsonArray>();
  JsonArray dailyWind = doc["daily"]["wind_speed_10m_max"].as<JsonArray>();
  JsonArray dailyGusts = doc["daily"]["wind_gusts_10m_max"].as<JsonArray>();
  JsonArray dailyCode = doc["daily"]["weather_code"].as<JsonArray>();

  if (dailyTime.size() > 0) {
    Serial.println("\n=========================================== DAILY FORECAST ==========================================");
    Serial.println("   Date       Max/Min Temp    PoP%   Precip    Rain    Snow    MaxWind   Gusts   Condition");
    Serial.println("-----------------------------------------------------------------------------------------------------");

    for (size_t i = 0; i < dailyTime.size(); i++) {
      const char *fullDate = dailyTime[i];
      const char *dateOnly = (strlen(fullDate) >= 10) ? (fullDate + 5) : fullDate;

      Serial_printf("   %-8s  %4.1f / %4.1f °F  %3d%%  %5.2f in %5.2f in %5.2f in %4.1f mph %4.1f mph  %s\n",
                    dateOnly,
                    dailyMax[i].as<float>(),
                    dailyMin[i].as<float>(),
                    dailyPop[i].as<int>(),
                    dailyPrecip[i].as<float>(),
                    dailyRain[i].as<float>(),
                    dailySnow[i].as<float>(),
                    dailyWind[i].as<float>(),
                    dailyGusts[i].as<float>(),
                    getWeatherDescription(dailyCode[i].as<int>()));
    }
  }
  Serial.println("================================================================================---------------------\n");
}

// Stream reader that strips HTTP headers and parses incoming payload
bool processIncomingStream(void (*outputFunc)(), bool process_incomplete) {
  while (client.available() > 0 && !headerComplete) {
    char c = client.read();
    switch (headerState) {
      case 0: headerState = (c == '\r') ? 1 : 0; break;
      case 1: headerState = (c == '\n') ? 2 : 0; break;
      case 2: headerState = (c == '\r') ? 3 : 0; break;
      case 3: headerState = (c == '\n') ? 4 : 0; break;
    }
    if (headerState == 4) {
      headerComplete = true;

      // Wait up to 1000ms for incoming payload bytes to buffer before deserializing
      uint32_t start = millis();
      while (client.available() == 0 && (millis() - start < 1000)) {
        delay(10);
      }

      doc.clear();
      DeserializationError error = deserializeJson(doc, client);
      if (error) serializeJsonPretty(doc, Serial1);
      if (!error || process_incomplete) {
        outputFunc();
      } else {
        Serial_printf("JSON Parsing failed: %s\n", error.c_str());
      }        
    }
  }

  if (!client.connected() && client.available() == 0) {
    client.stop();
    return true;  // Stream fully processed
  }
  return false;
}

// -------------------------------------------------------------------
// print out WiFi status
// -------------------------------------------------------------------
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// -------------------------------------------------------------------
// Setup & Loop
// -------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000)
    ;
  Serial1.begin(115200);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  // attempt to connect to Wifi network:
  while (true) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 3 seconds for connection:
    if (status == WL_CONNECTED) break;
    delay(3000);
  }
  Serial.println("Connected to wifi");

  printWifiStatus();

  appState = FETCH_MAP_CITY_TO_LOCATION;
}

void loop() {
  switch (appState) {
    case FETCH_MAP_CITY_TO_LOCATION:
      Serial.println("\n[0/3] Map City to Location...");
      if (sendMapCityRequest()) {
        appState = READ_MAP_CITY_TO_LOCATION;
      } else {
        Serial.println("Connection failed.");
        appState = DONE;
      }
      break;
    case READ_MAP_CITY_TO_LOCATION:
      if (processIncomingStream(printMapCityData, true)) {
        appState = FETCH_CURRENT;
      }
      break;
    case FETCH_CURRENT:
      Serial.println("\n[1/3] Requesting Current Weather...");
      if (sendCurrentRequest()) {
        appState = READ_CURRENT;
      } else {
        Serial.println("Connection failed.");
        appState = DONE;
      }
      break;

    case READ_CURRENT:
      if (processIncomingStream(printCurrentData, false)) {
        appState = FETCH_HOURLY;
      }
      break;

    case FETCH_HOURLY:
      Serial.println("\n[2/3] Requesting Hourly Forecast...");
      if (sendHourlyRequest()) {
        appState = READ_HOURLY;
      } else {
        Serial.println("Connection failed.");
        appState = DONE;
      }
      break;

    case READ_HOURLY:
      if (processIncomingStream(printHourlyData, false)) {
        appState = FETCH_DAILY;
      }
      break;

    case FETCH_DAILY:
      Serial.println("\n[3/3] Requesting Daily Forecast...");
      if (sendDailyRequest()) {
        appState = READ_DAILY;
      } else {
        Serial.println("Connection failed.");
        appState = DONE;
      }
      break;

    case READ_DAILY:
      if (processIncomingStream(printDailyData, false)) {
        Serial.println("All data successfully fetched!");
        Serial.println("Enter City name:");
        appState = DONE;
      }
      break;

    case DONE:
      break;

    default:
      break;
  }

  if (Serial.available()) {
    weather_city = Serial.readString();
    weather_city.trim();
    weather_city.replace(' ', '+');
    Serial.print("New City: ");
    Serial.println(weather_city);
    appState = FETCH_MAP_CITY_TO_LOCATION;
  }
}