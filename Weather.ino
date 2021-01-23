
#include "ani.h"

#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson.git

#include "NTPClient.h"           //https://github.com/taranais/NTPClient

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define darkgreen 0xA041
#define blue 0x5D9B
#include "Orbitron_Medium_20.h"

#include <WiFi.h>

#include <WiFiUdp.h>

#include <HTTPClient.h>

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;

const char * ssid = "";
const char * password = "";
String town = "Dhaka";
String Country = "BD";

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=Dhaka,BD&units=metric&APPID=";
const String aqiendpoint = "http://api.waqi.info/feed/dhaka/?token=";

String payload = ""; //whole json 
String aqipayload = ""; //whole json 
String tmp = ""; //temperatur
String hum = ""; //humidity
String prs = ""; //pressure
String weathr = ""; //weather  
String aqi = ""; //air quality index 

StaticJsonDocument < 6000 > doc;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

int backlight[5] = {
    10,
    30,
    60,
    120,
    220
};
byte b = 3;

word RGB(byte R, byte G, byte B) {
    return (((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3));
}

void setup(void) {
    pinMode(0, INPUT_PULLUP);
    pinMode(35, INPUT);
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
    ledcAttachPin(TFT_BL, pwmLedChannelTFT);
    ledcWrite(pwmLedChannelTFT, backlight[b]);

    Serial.begin(115200);
    tft.print("Connecting to ");
    tft.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        tft.print(".");
    }

    tft.println("");
    tft.println("WiFi connected.");
    tft.println("IP address: ");
    tft.println(WiFi.localIP());
    delay(3000);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);
    tft.setCursor(2, 232, 1);
    tft.println(WiFi.localIP());
    

    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setCursor(70, 152, 2);
    tft.println("AirQuality");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setCursor(4, 152, 2);
    tft.println("Temp.C");

    tft.setCursor(4, 194, 2);
    tft.println("Humidity");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);

    tft.setCursor(70, 194, 2);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.println("Baro.HPa");


    //tft.setTextSize(2);


    tft.setFreeFont(&Orbitron_Medium_20);
    //tft.setCursor(6, 86);
    tft.setCursor(6, 62);
    tft.println(town);

    //tft.fillRect(60, 152, 1, 74, TFT_GREY); // vertical lilne


    //for (int i = 0; i < b + 1; i++)
    //    tft.fillRect(78 + (i * 7), 216, 3, 10, blue);

    // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +8 = 28800 //kl
    // GMT +6 = 21600 //dhaka
    timeClient.setTimeOffset(21600); /*EDDITTTTTTTTTTTTTTTTTTTTTTTT                      */
    getData();
    delay(500);
}

//-------------------------------------------------------------------
int i = 0;
String tt = "";
int count = 0;
bool inv = 1;
int press1 = 0;
int press2 = 0; ////
int frame = 0;
String curSeconds = "";
//-------------------------------------------------------------------

void loop() {

    tft.pushImage(0, 88, 135, 65, ani[frame]);
    frame++;
    if (frame >= 10)
        frame = 0;

    if (digitalRead(35) == 0) {
        if (press2 == 0) {
            press2 = 1;
            tft.fillRect(78, 216, 44, 12, TFT_BLACK);

            b++;
            if (b >= 5)
                b = 0;

            for (int i = 0; i < b + 1; i++)
                tft.fillRect(78 + (i * 7), 216, 3, 10, blue);
            ledcWrite(pwmLedChannelTFT, backlight[b]);
        }
    } else press2 = 0;

    if (digitalRead(0) == 0) {
        if (press1 == 0) {
            press1 = 1;
            inv = !inv;
            tft.invertDisplay(inv);
        }
    } else press1 = 0;

    if (count == 0)
        getData();
    count++;
    if (count > 2000)
        count = 0;

    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(2, 187);
    tft.println(tmp.substring(0, 3));

    tft.setCursor(2, 227);
    tft.println(hum + "%");

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(70, 227, 1);
    tft.fillRect(70, 210, 130, 20, TFT_BLACK);
    tft.println(prs);


    tft.setTextColor(RGB(0xA0, 0xC0, 0x70), TFT_BLACK);
    tft.setTextFont(2);
    //tft.setCursor(6, 44);
    tft.setCursor(6, 69);
    tft.println(weathr);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);



    while (!timeClient.update()) {
        timeClient.forceUpdate();
    }

    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();
    //Serial.println(formattedDate);

    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);

    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);

    if (curSeconds != timeStamp.substring(6, 8)) {
        //tft.fillRect(78,170,48,28,darkred);

        unsigned long aqiColor = 0xFFFFFF;
        if (aqi.toInt() <= 50) aqiColor = RGB(0x00, 0x99, 0x66);;
        if (aqi.toInt() > 50 && aqi.toInt() < 100) aqiColor = RGB(0xFF, 0xDE, 0x33);
        if (aqi.toInt() > 100 && aqi.toInt() < 150) aqiColor = RGB(0xFF, 0x99, 0x33);
        if (aqi.toInt() > 150 && aqi.toInt() < 200) aqiColor = RGB(0xCC, 0x00, 0x33);
        if (aqi.toInt() > 200 && aqi.toInt() < 300) aqiColor = RGB(0x66, 0x00, 0x99);
        if (aqi.toInt() > 300) aqiColor = RGB(0x7E, 0x00, 0x23);

        tft.fillRect(68, 170, 67, 23, aqiColor);
        if (aqi.toInt() >= 300) delay(50);
        tft.setFreeFont(&Orbitron_Medium_20);
        tft.setCursor(75, 190);
        tft.println(aqi);
        tft.fillRect(60, 152, 1, 74, TFT_GREY); // vertical lilne
        curSeconds = timeStamp.substring(6, 8);
    }

    tft.setFreeFont(&Orbitron_Light_32);
    String current = timeStamp.substring(0, 5);
    if (current != tt) {
        tft.fillRect(3, 8, 120, 30, TFT_BLACK);
        tft.setCursor(5, 34);
        tft.println(timeStamp.substring(0, 5));
        tt = timeStamp.substring(0, 5);
    }

    delay(80);
}

void getData() {
    tft.fillRect(1, 170, 64, 20, TFT_BLACK);
    tft.fillRect(1, 210, 64, 20, TFT_BLACK);
    if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
        HTTPClient http;
        http.begin(endpoint); //Specify the URL
        int httpCode = http.GET(); //Make the request
        if (httpCode > 0) { //Check for the returning code
            payload = http.getString();
            // Serial.println(httpCode);
            Serial.println(payload);
        } else {
            Serial.println("Error on HTTP request");
        }

        http.end(); //Free the resources
        HTTPClient http2;
        http2.begin(aqiendpoint); //Specify the URL
        httpCode = http2.GET(); //Make the request
        if (httpCode > 0) { //Check for the returning code
            aqipayload = http2.getString();
            Serial.println(aqipayload);
        } else {
            Serial.println("Error on HTTP request");
        }
        http2.end(); //Free the resources
    }
    char inp[6000];
    payload.toCharArray(inp, 1000);
    deserializeJson(doc, inp);

    String tmp2 = doc["main"]["temp"];
    String hum2 = doc["main"]["humidity"];
    String prs2 = doc["main"]["pressure"];
    String town2 = doc["name"];
    String weathr2 = doc["weather"][0]["description"];

    aqipayload.toCharArray(inp, 1000);
    deserializeJson(doc, inp);
    String aqi2 = doc["data"]["aqi"];

    prs=prs2;
    aqi = aqi2;
    weathr = weathr2;
    tmp = tmp2;
    hum = hum2;
    dayStamp = weathr;

    Serial.println("Temperature" + String(tmp));
    Serial.println("Humidity" + hum);
    Serial.println("Weather " + weathr);
    Serial.println(town);

}
