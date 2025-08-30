
#if defined(ESP32)
#include <ESP8266WiFi.h>


#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)

#include <FirebaseESP8266.h>
#endif

#include "CTBot.h"
#include <DHT.h>  // Including library for dht
CTBot myBot;
TBMessage msg;

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>


//Wifi Cedentials

const char* ssid = "XXXXXXXXXXX";  // your SSID Wifi name

const char* password = "xxxxxxxxxxxx"; // your Wifi password

String token = "XXXXXXXXXXXXXXXXXXXXXX"   ; // Enter Your Telegram Bot Token
uint32_t ChatId = XXXXXXXXX ; // Your own telegram chat id from IDBot

/* 2. Define the API Key */
#define API_KEY "XXXXXXXXXXXXXXXXXXXXXXXXX " // Firebase Api Key

/* 3. Define the RTDB URL */
#define DATABASE_URL "example-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOk = false ;
//Thingspeak


String apiKey = "XXXXXXXXXXXXXXX "; // Thinkspeak API key

const char* server = "api.thingspeak.com";




WiFiClient client;

#define DHTPIN 2         //pin where the dht11 is connected
 DHT dht(DHTPIN, DHT22);//DHT

float Level;  // MQ2 

int Flame ; // Flame 
 
int Ldr ; // Ldr 



// FIREBASE ALERT STRINGS 

String SmokeAlert = "Smoke Alert";
String LightAlert = "High Light Detected";
String TemperatureAlert= "High Temperature Detected" ;
String FlameAlert="Fire Detected" ;
String Normal="Condition Normal" ;





void setup()

{

Serial.begin(115200);

delay(10);

dht.begin(); //DHT BEGIN 

Serial.println("Connecting to ");

Serial.println(ssid);

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED)

{

delay(500);

Serial.print(".");

}

Serial.println("");

Serial.println("WiFi connected");
Serial.print("Connected with IP: ");
Serial.println(WiFi.localIP());
Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
 
pinMode(0, OUTPUT);  // BUZZER 

pinMode(4, INPUT); // flame sensor 

pinMode(5, INPUT);  // lDR Sensor 

/* Assign the api firebase key (required) */
config.api_key = API_KEY;

/* Assign the RTDB URL (required) */
config.database_url = DATABASE_URL;

/* Assign the callback function for the long running token generation task */
config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
 //////////////////////////////////////////
 
  if(Firebase.signUp(&config,&auth,"",""))
  {
     Serial.println("ok");
     signupOk = true ;  
  }
  else
  {
    Serial.println("%s\n Hbe na hbe na");
  }

   Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
 // Firebase.reconnectWiFi(true);

  int n=0;
  Firebase.setDoubleDigits(5);

//...............................
myBot.wifiConnect(ssid,password);
  myBot.setTelegramToken(token);
  if (myBot.testConnection()){
   
    Serial.println("\ntestConnection OK");
  }
  else
  {
    Serial.println("\ntestConnection NOK");
  }
  
 
}

void loop()


{

// LDR SENSOR 
Ldr = digitalRead(5);
// FLAME SENSOR 
Flame = digitalRead(4);



//DHT11 
 float h = dht.readHumidity();
      float t = dht.readTemperature();
      
              if (isnan(h) || isnan(t)) 
                 {
                     Serial.println("Failed to read from DHT sensor!");
                      return;
                 }
//MQ2 
float reading = analogRead(A0);

Level = ((reading/1023)*100);

if (isnan(Level))

{

Serial.println("Failed to read from MQ-2 sensor!");

return;

}

 

if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com

{
    //DHT22
    String postStr = apiKey;

    postStr +="&field1=";

    postStr += String(Level);

   // postStr += "\r\n\r\n";
   
    //MQ2 
     postStr +="&field2=";
     postStr += String(t);
     postStr +="&field3=";
     postStr += String(h);
    // postStr += "\r\n\r\n";
      postStr +="&field4=";
     postStr += String(Flame);
     //postStr += "\r\n\r\n";
      postStr +="&field5=";
     postStr += String(Ldr);
   //  postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");

    client.print("Host: api.thingspeak.com\n");

    client.print("Connection: close\n");

    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");

    client.print("Content-Type: application/x-www-form-urlencoded\n");

    client.print("Content-Length: ");

    client.print(postStr.length());

    client.print("\n\n");

    client.print(postStr);

Serial.print("Gas, Smoke, Air Level: ");
Serial.println(Level);

Serial.print("Temperature: ");
Serial.print(t);
Serial.print("\n Degrees Celcius, Humidity: ");
Serial.print(h);

Serial.print("\n Flame: ");
Serial.print(Flame);

Serial.print("\n LDR : ");
Serial.print(Ldr);

Serial.println("\n Data Send to Thingspeak");

}

delay(500);

client.stop();

Serial.println("Waiting...");

 

// thingspeak needs minimum 15 sec delay between updates.

delay(1500);


//Firebase Code //
if(Firebase.ready()&& signupOk){

  //Temperature 
  if(Firebase.RTDB.setInt(&fbdo,"esp/Temperature",t))
      {
        //Serial.println("Passed");
       // intValue=fbdo.intData();
        //Serial.println("Type:"+ fbdo.dataType());
        //Serial.println(intValue);
      }

  // Humidity 
  if(Firebase.RTDB.setInt(&fbdo,"esp/Humidity",h))
      {
       // Serial.println("Passed");
       // intValue=fbdo.intData();
       // Serial.println("Type:"+ fbdo.dataType());
        //Serial.println(intValue);
      }

  // Smoke 
  if(Firebase.RTDB.setInt(&fbdo,"esp/Smoke",Level))
      {
        //Serial.println("Passed");
      //  intValue=fbdo.intData();
        //Serial.println("Type:"+ fbdo.dataType());
        //Serial.println(intValue);
      }

  //Light 
  if(Firebase.RTDB.setInt(&fbdo,"esp/Light",Ldr))
      {
      //  Serial.println("Passed");
        //intValue=fbdo.intData();
       // Serial.println("Type:"+ fbdo.dataType());
       // Serial.println(intValue);
      }
  // Flame 
  if(Firebase.RTDB.setInt(&fbdo,"esp/Flame",Flame))
      {
        //Serial.println("Passed");
       // intValue=fbdo.intData();
        //Serial.println("Type:"+ fbdo.dataType());
        //Serial.println(intValue);
      }

}

//BUZZER CODE 
if (Level>35)
{ 
  Firebase.RTDB.setString(&fbdo,"esp/Buzzer",SmokeAlert);
      
  tone(0, 3000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(0);     // Stop sound...
  delay(1000);        // ...for 1sec
  //............................................
    Serial.print("Sending a message via CTBot");
    Serial.println(" with bot token " + token);
    Serial.println(myBot.sendMessage(ChatId, "SMOKE DETECTED!"));
  
}

else
{
   Firebase.RTDB.setString(&fbdo,"esp/Buzzer",Normal);
}

if (t>40)
{ 
  Firebase.RTDB.setString(&fbdo,"esp/Buzzer",TemperatureAlert);
  Serial.println("High Tempertature Detected!!!");
  tone(0, 3000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(0);     // Stop sound...
  delay(1000);        // ...for 1sec
  //............................................
    Serial.print("Sending a message via CTBot");
    Serial.println(" with bot token " + token);
    Serial.println(myBot.sendMessage(ChatId, "HIGH TEMPERATURE DETECTED!"));

}
else
{
   Firebase.RTDB.setString(&fbdo,"esp/Buzzer",Normal);
}

if (Flame==0)
  {
    Firebase.RTDB.setString(&fbdo,"esp/Buzzer",FlameAlert);
    Serial.println("Fire!!!");
   // digitalWrite(0, HIGH);
    tone(0, 2000); // Send 1KHz sound signal...
    delay(1000);        // ...for 1 sec
    //............................................
    Serial.print("Sending a message via CTBot");
    Serial.println(" with bot token " + token);
    Serial.println(myBot.sendMessage(ChatId, "FIRE DETECTED!"));
   
  }
  else
  {
     Firebase.RTDB.setString(&fbdo,"esp/Buzzer",Normal);
    Serial.println("No worries");
    //digitalWrite(0, LOW);
     noTone(0);     // Stop sound...
    delay(1000);        // ...for 1sec
  }
if (Ldr==0)
  {
    Firebase.RTDB.setString(&fbdo,"esp/Buzzer",LightAlert);
    Serial.println("High Light Detected !!!");
    digitalWrite(0, HIGH);
    //............................................
    Serial.print("Sending a message via CTBot");
    Serial.println(" with bot token " + token);
    Serial.println(myBot.sendMessage(ChatId, "HIGH LIGHT DETECTED!"));
  }
  else
  {

     Firebase.RTDB.setString(&fbdo,"esp/Buzzer",Normal);
    digitalWrite(0, LOW);
  }
}
