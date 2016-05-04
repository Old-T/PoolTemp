//
// This sketch uses an ESP8266-based Adafruit Feather Huzzah for measuring
// temperature and humidity with a DHT11 sensor
// The values are sent as feeds to an Adafruit IO account via Adafruit MQTT publish calls
//
// To save power, deep sleep from the ESP8266 library is used
// To wake up the board a signal on pin 16 is sent to the reset input
// (so you need a wire between D16 and the reset pin)
// Note! The connection D16 -> Reset has to be removed when uploading the sketch!
// All activity happens in the setup() method as the board is reset after sleeping
//

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Your personal WiFi- and Adafruit IO credentials
// Should define WLAN_SSID, WLAN_PASS and AIO_KEY and AIO_USERNAME
#include "WIFI_and_Adafruit_IO_parameters.h"

//
// Adafruit IO setup
//
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883

WiFiClient client;

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

const char POOL_TEMP_FEED[] PROGMEM = AIO_USERNAME "/feeds/Pooltemp";
Adafruit_MQTT_Publish pooltempPub = Adafruit_MQTT_Publish(&mqtt, POOL_TEMP_FEED);

const char POOLHOUSE_TEMP_FEED[] PROGMEM = AIO_USERNAME "/feeds/Pooltak";
Adafruit_MQTT_Publish poolhousetempPub = Adafruit_MQTT_Publish(&mqtt, POOLHOUSE_TEMP_FEED);

//
// Sensor setup
//
#include <OneWire.h>
#include <DallasTemperature.h>
#define OUTDOOR_MEASUREMENT     4
#define POOLTEMP_MEASUREMENT    5
// Setup for DS18B20
#define ONE_WIRE_BUS 4                // Uses digital pin 3
#define ONE_WIRE_BUS2 5                // Uses digital pin 4
//#include <DHT.h>
//#define DHTPIN 12       // Digital input pin for DHT11
//#define DHTTYPE DHT11
//DHT dht(DHTPIN, DHTTYPE);
OneWire ourWire(ONE_WIRE_BUS);
OneWire ourWire2(ONE_WIRE_BUS2);
DallasTemperature PoolHouseTemp(&ourWire);
DallasTemperature PoolTemp(&ourWire2);

#define SLEEP_SECONDS 600

void MQTT_connect()
{
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  int8_t ret;
  while ((ret = mqtt.connect()) != 0)
  {
    // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");
}

void sendDataToAdafruitIO(float pooltemp, float poolhousetemp)
{
  unsigned long startTime = millis();

  Serial.println("Start sending data to Adafruit IO...");

  MQTT_connect();

  if (pooltempPub.publish(pooltemp))
  {
    Serial.println(pooltemp);
    Serial.println("Sent temperature ok");
  }
  else
  {
    Serial.println("Failed sending pooltemp");
  }

  if (poolhousetempPub.publish(poolhousetemp))
  {
    Serial.println(poolhousetemp);
    Serial.println("Sent poolhouse temp ok");
  }
  else
  {
    Serial.println("Failed sending poolhousetemp");
  }

  unsigned long endTime = millis();

  Serial.print("Sending data took ");
  Serial.print((endTime - startTime) / 1000.0);
  Serial.println(" second(s)");
}

void setupWiFi()
{
  unsigned long startTime = millis();

  // Setup serial port access.
  Serial.begin(115200);

  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  unsigned long endTime = millis();

  Serial.print("Setting up WiFi took ");
  Serial.print((endTime - startTime) / 1000.0);
  Serial.println(" second(s)");

}

void setup()
{
  setupWiFi();

  //  dht.begin();

  PoolHouseTemp.requestTemperatures();
  float poolHouseTemp = PoolHouseTemp.getTempCByIndex(0);
  if (poolHouseTemp < 0)
  {
      PoolHouseTemp.requestTemperatures();
      poolHouseTemp = PoolHouseTemp.getTempCByIndex(0);
  }
  
  Serial.print("PoolHouseTemp ");
  Serial.print(poolHouseTemp);
  Serial.println(" grader");

  PoolTemp.requestTemperatures();
  float poolTemp = PoolTemp.getTempCByIndex(0);
  if (poolTemp < 0)
  {
      PoolTemp.requestTemperatures();
      poolTemp = PoolHouseTemp.getTempCByIndex(0);
  }
  Serial.print("PoolTemp ");
  Serial.print(poolTemp);
  Serial.println(" grader");

  sendDataToAdafruitIO(poolTemp, poolHouseTemp);

  // Put the board to deep sleep to save power. Will send a signal on D16 when it is time to wake up.
  // Thus, connect D16 to the reset pin. After reset, setup will be executed again.
  Serial.print("Going to deep sleep for ");
  Serial.print(SLEEP_SECONDS);
  Serial.println(" seconds");
  ESP.deepSleep(SLEEP_SECONDS * 1000000);
}

void loop()
{
  // nothing to do here as setup is called when waking up after deep sleep
}

