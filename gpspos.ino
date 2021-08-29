#include <SoftwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
  
const char* ssid = "ssid";
const char* password =  "pass";
const char* URL =  "posturl";   
SoftwareSerial mySerial(32, 26);
 
// NMEAの緯度経度を「度分秒」(DMS)の文字列に変換する
String NMEA2DMS(float val) {
  int d = val / 100;
  int m = ((val / 100.0) - d) * 100.0;
  float s = ((((val / 100.0) - d) * 100.0) - m) * 60;
  return String(d) + "度" + String(m) + "分" + String(s, 1) + "秒";
}
 
// (未使用)NMEAの緯度経度を「度分」(DM)の文字列に変換する
String NMEA2DM(float val) {
  int d = val / 100;
  float m = ((val / 100.0) - d) * 100.0;
  return String(d) + "度" + String(m, 4) + "分";
}
 
// NMEAの緯度経度を「度」(DD)の文字列に変換する
String NMEA2DD(float val) {
  int d = val / 100;
  int m = (((val / 100.0) - d) * 100.0) / 60;
  float s = (((((val / 100.0) - d) * 100.0) - m) * 60) / (60 * 60);
  return String(d + m + s, 6);
}
 
// UTC時刻から日本の標準時刻に変換する(GMT+9:00)
String UTC2GMT900(String str) {
  int hh = (str.substring(0,2).toInt()) + 9;
  if(hh > 24) hh = hh - 24;
 
  return String(hh,DEC) + ":" + str.substring(2,4) + ":" + str.substring(4,6);  
}
void svrPOST(String times,String lat,String lon){
 if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
   HTTPClient http;   
   http.begin(URL);  //Specify destination for HTTP request
   http.addHeader("Content-Type", "text/plain");             //Specify content-type header
   String senddata = "{ \"times\" :";
               senddata +=  "\"";
               senddata += times;
               senddata +=  "\",";
               senddata += "\"lat\" :";
               senddata +=  "\"";
               senddata += lat;
               senddata +=  "\",";
               senddata += "\"lon\" :";
               senddata +=  "\"";
               senddata += lon;
               senddata +=  "\"";
               senddata += "}";
   int httpResponseCode = http.POST(senddata);   //Send the actual POST request
   if(httpResponseCode>0){
    String response = http.getString();                       //Get the response to the request
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
   }else{
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
   }
   http.end();  //Free resources
 }else{
    Serial.println("Error in WiFi connection");   
 }
 
}
void setup() {
  mySerial.begin(9600);
  Serial.begin(115200);
  delay(4000);   //Delay needed before calling the WiFi.begin
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}
void loop() {
  // 1つのセンテンスを読み込む
  String line = mySerial.readStringUntil('\n');
 
  if(line != ""){
    int i, index = 0, len = line.length();
    String str = "";
  
    // StringListの生成(簡易)
    String list[30];
    for (i = 0; i < 30; i++) {
      list[i] = "";
    }
 
    // 「,」を区切り文字として文字列を配列にする
    for (i = 0; i < len; i++) {
      if (line[i] == ',') {
        list[index++] = str;
        str = "";
        continue;
      }
      str += line[i];
    }
    
    // $GPGGAセンテンスのみ読み込む
    if (list[0] == "$GPGGA") {
      
      // ステータス
      if(list[6] != "0"){      
        svrPOST(String(UTC2GMT900(list[1])),String(NMEA2DD(list[2].toFloat())),String(NMEA2DD(list[4].toFloat())));
      }else{
        Serial.print("測位できませんでした。");
      }
      
      Serial.println("");
    }
  }
}
