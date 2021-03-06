#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#ifndef STASSID
// #define STASSID "Kalathiparambil"
// #define PASSWORD "Abin@1999"
#define LED LED_BUILTIN
#define H HIGH
#define L LOW
#define STASSID "abjs"
#define PASSWORD "abinjoseph"
#define HOST "covid-19-tracker-abjs.vercel.app"
#define HOST_FINGERPRINT "1fa81791acb6d1b94ea2241d6461daad88157c5e"
#endif
WiFiClientSecure client;
const char *ssid = STASSID;
const char *password = PASSWORD;
String outputLineTow;
int Cases, Deaths, Recovered, Active;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void makeHTTPRequest()
{

  if (!client.connect(HOST, 443))
  {
    Serial.println(F("Connection failed"));
    return;
  }

  // give the esp a breather
  yield();

  client.print(F("GET "));
  client.print("/api/india");
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(HOST);
  client.println(F("Cache-Control: no-cache"));
  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }
  //delay(100);
  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
    Serial.print(c);
    Serial.println("BAD");
  }

  //  // While the client is still availble read each
  //  // byte and print to the serial monitor
  //  while (client.available()) {
  //    char c = 0;
  //    client.readBytes(&c, 1);
  //    Serial.print(c);
  //  }

  //Use the ArduinoJson Assistant to calculate this:

  //StaticJsonDocument<192> doc;
  DynamicJsonDocument doc(192); //For ESP32/ESP8266 you'll mainly use dynamic.

  DeserializationError error = deserializeJson(doc, client);

  if (!error)
  {
    Cases = doc["Cases"];
    Deaths = doc["Deaths"];
    Recovered = doc["Recovered"];
    Active = doc["Active"];
    outputLineTow[0] = '\0';
    outputLineTow += "Deaths :";
    outputLineTow += Deaths;
    outputLineTow += " Recovered :";
    outputLineTow += Recovered;
    outputLineTow += " Active :";
    outputLineTow += Active;
    outputLineTow += " " + outputLineTow;
    Serial.print("Cases: ");
    Serial.println(Cases);
    Serial.print(" Deaths: ");
    Serial.println(Deaths);
    Serial.print(" Recovered: ");
    Serial.println(Recovered);
    Serial.print(" Active: ");
    Serial.println(Active);
    Serial.println(outputLineTow);
  }
  else
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
}
void scrollText(int row, String message, int delayTime, int lcdColumns)
{
  for (int i = 0; i < lcdColumns; i++)
  {
    message = " " + message;
  }
  message = message + " ";
  for (int pos = 0; pos < message.length(); pos++)
  {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
  for (int pos = 0; pos < message.length(); pos++)
  {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}
void setup()
{
  delay(2000);
  pinMode(LED_BUILTIN, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Covid-19 INDIA");
  lcd.setCursor(1, 1);
  lcd.print("Viral Science");
  Serial.begin(9600);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    digitalWrite(LED_BUILTIN, H);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, L);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WIFI CONNECTED");
    lcd.setCursor(0, 1);
    lcd.print("IP:");
    lcd.setCursor(3, 1);
    lcd.print(WiFi.localIP());
    Serial.println("Connected to the WiFi network");
    Serial.println(WiFi.localIP());
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Covid-19 INDIA");
    lcd.setCursor(1, 1);
    lcd.print("Viral Science");
  }
  // client.setFingerprint(HOST_FINGERPRINT);
  client.setInsecure();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, L);
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, H);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WIFI DISCONNECTED");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    delay(3000);
    digitalWrite(LED_BUILTIN, L);
    delay(2000);
    Serial.println("Reconnecting to WiFi..");
  }
  makeHTTPRequest();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cases:");
  lcd.setCursor(7, 0);
  lcd.print(Cases);
  lcd.setCursor(0, 1);
  scrollText(1, outputLineTow, 750, lcdColumns);
  delay(20000);
}
