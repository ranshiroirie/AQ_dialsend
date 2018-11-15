#include <WiFi.h>
#include <HTTPClient.h>

#define SSID "aqwlan-guest"
#define PASSWORD "AqGuest01"
#define BASEURL "http://52.197.122.211/api/dices"

HardwareSerial TWE(0); // RX, TX

String ID, DIAL;

bool send_trigger = false;

void setup() {
  TWE.begin(38400);
  Serial.begin(38400);
  while (!Serial);

  WiFi.begin(SSID, PASSWORD);
  Serial.printf("Connecting to the WiFi AP: %s ", SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  receiving();
  if (send_trigger == true && ID.length() == 8)
  {
    DIAL_convert(DIAL);
    if (DIAL != "E") {
      send_to_http(ID, DIAL);
    }
    send_trigger = false;
  }
}

void receiving() {
  byte recv [60] = {};
  byte count     = 0;

  while (TWE.available())
  {
    recv [count] = TWE.read();
    count++;
  }

  if (count == 51)
  {
    byte space [] = {1, 3, 5, 7, 9, 11, 19, 21, 25, 27, 31, 33, 35, 37, 47, 49};
    byte add      = 0;
    String rawid = "";
    String rawdial = "";

    for (byte i = 0 ; i < count ; i++)
    {
      if (i == space[add])
      {
        Serial.print(" ");
        add++;
      }

      if (recv[i] == ':')
      {
        Serial.print(':');
      } else if (recv[i] == 13) {//:
        Serial.print(13, HEX);
      } else if (recv[i] == 10) {//改行
        Serial.println(10, HEX);
      } else {
        if (recv[i] <= '9') recv[i] = recv[i] - '0';
        else if (recv[i] <= 'F') recv[i] = recv[i] - 'A' + 10;

        if (11 <= i && i <= 18) {
          rawid += String(recv[i], HEX);
        }
        if (i == 33 || i == 34) {
          rawdial += String(recv[i], HEX);
        }

        Serial.print(recv[i], HEX);
      }
    }
    rawid.toUpperCase();
    rawdial.toUpperCase();
    Serial.print("識別番号:");
    Serial.println(rawid);
    ID = rawid;
    DIAL = rawdial;
    send_trigger = true;
  }

}

void DIAL_convert(String dial) {
  if (dial == "0F") {
    DIAL = "0";
  } else if (dial == "0E") {
    DIAL = "1";
  } else if (dial == "0D") {
    DIAL = "2";
  } else if (dial == "0C") {
    DIAL = "3";
  } else if (dial == "0B") {
    DIAL = "4";
  } else {
    DIAL = "E";
  }
  Serial.print("値:");
  Serial.println(DIAL);
}

void send_to_http(String id, String dial) {
  char http_data[100];
  char buf_id[8];
  char buf_dial[1];

  id.toCharArray(buf_id, 9);
  dial.toCharArray(buf_dial, 2);
  sprintf(http_data, "dice_id=%s&angle=%s", buf_id, buf_dial);

  Serial.print("Post:");
  Serial.print(http_data);
  Serial.println("");
  HTTPClient http;
  http.begin(BASEURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); ///POSTで送る時のファイルのデータの種類の項目
  int httpCode = http.POST(http_data);
  Serial.printf("Response: %d", httpCode);
  Serial.println();
}

