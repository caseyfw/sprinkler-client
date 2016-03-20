#include <ESP8266WiFi.h>

// Change these to suit your setup.
const char ssid[] = "Your Wifi Network";
const char password[] = "your-wifi-password";
const char hostname[] = "server.hostname";
const char path[] = "/api/v1";

// This is the ID that's used when fetching instructions.
const char sprinklerId[] = "1";

const unsigned long sleepTime = 60;

int controlA = 12;
int controlB = 14;

WiFiClient client;
String response;

void setSprinkler(bool setting);

void setup()
{
  Serial.begin(9600);
  // Set solenoid control pins to output mode.
  pinMode(controlA, OUTPUT);
  pinMode(controlB, OUTPUT);

  Serial.print("Joining Wifi network... ");
  // Join wifi network.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("Done.");

  Serial.print("Connecting to server... ");
  // Connect to remote host.
  while (!client.connect(hostname, 80)) {
    delay(100);
  }
  Serial.println("Done.");

  Serial.println("Issuing request.");
  // Issue request.
  client.print("GET ");
  client.print(path);
  client.print("/sprinkler/");
  client.print(sprinklerId);
  client.println("/instruction HTTP/1.1");
  client.print("Host: ");
  client.println(hostname);
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
}

void loop()
{
  // If there are incoming bytes available from the server, read them.
  while (client.available()) {
    char c = client.read();

    if (response.length() < 1000) {
      response += c;
    }
  }

  // If the server has disconnected, stop the client, do action and go to sleep.
  if (!client.connected()) {
    Serial.println("Server has disconnected.");
    Serial.println("Response:");
    Serial.println(response);

    client.stop();

    if (response.indexOf("turn_on") > 0) {
      Serial.println("Turning sprinkler on.");
      setSprinkler(true);
    } else if (response.indexOf("turn_off") > 0) {
      Serial.println("Turning sprinkler off.");
      setSprinkler(false);
    } else {
      Serial.println("Null response.");
    }

    Serial.println("Sleeping.");
    delay(500);
    Serial.end();
    ESP.deepSleep(sleepTime * 1000000);
  }
}

void setSprinkler(bool setting)
{
  if (setting) {
    digitalWrite(controlA, HIGH);
    digitalWrite(controlB, LOW);
  } else {
    digitalWrite(controlA, LOW);
    digitalWrite(controlB, HIGH);
  }

  delay(250);

  digitalWrite(controlA, LOW);
  digitalWrite(controlB, LOW);
}
