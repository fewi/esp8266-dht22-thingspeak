#include <ESP8266WiFi.h>
#include <DHT.h>
#include <WiFiClient.h>
#include "credentials.h"

#define DHTTYPE DHT22
#define DHTPIN  D4

#define SLEEP_DELAY_IN_SECONDS  60

#define THING_SPEAK_ADDRESS  "api.thingspeak.com"

DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

float humidity, temp_c;  // Values read from sensor
WiFiClient client;

void setup(void)
{
  Serial.begin(115200); 
  dht.begin();           // initialize temperature sensor

  // Connect to WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("\n\r \n\rConnecting to WIFI network ");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

}

void loop(void)
{
  if (getTemperature()) {
    transferData();
  }
  delay(2000);
  
  ESP.deepSleep(SLEEP_DELAY_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);
}

boolean getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)

  humidity = dht.readHumidity();          // Read humidity (percent)
  temp_c = dht.readTemperature(false);     // Read temperature as Fahrenheit

  if (isnan(humidity) || isnan(temp_c)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }
  else {
    Serial.print("temperature: ");
    Serial.print(temp_c);
    Serial.println(" C");

    Serial.print("humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    return true;
  }
}

void transferData() {
  String url = "/update?api_key=" + API_KEY + "&field1=" + temp_c + "&field2=" + humidity;
  if (!client.connect(THING_SPEAK_ADDRESS, 80)) {
    Serial.println("connection to ThingSpeak failed");

  } else {
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host:" + THING_SPEAK_ADDRESS + "\r\n" + "Connection: keep-alive\r\n\r\n");

    while (!client.available()) {
      //waiting...
    }

    delay(200);
  }

  if (getStatus())
    Serial.println("Data transfered successfully!");
}


boolean getStatus() {
  bool stat;
  String _line;

  _line = client.readStringUntil('\n');
  int separatorPosition = _line.indexOf("HTTP/1.1");

  if (separatorPosition >= 0) {
    if (_line.substring(9, 12) == "200")
      stat = true;
    else
      stat = false;

    return  stat;
  }
}

