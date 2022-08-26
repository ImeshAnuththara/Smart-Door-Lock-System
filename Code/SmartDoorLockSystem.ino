#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FirebaseArduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Keypad.h>
#define LOCK D8

#define FIREBASE_HOST "smartdoorlocksystem-cecd3-default-rtdb.firebaseio.com"       // the project name address from firebase id
#define FIREBASE_AUTH  "LglXMVZjv1zuK6j5FaaQ6mw95B8kETfEWnggH9WA"       // the secret key generated from firebase

const char* ssid     = "Dialog 4G";
const char* password = "G68D0RFAEJT";
const char* server ="broker.hivemq.com";//hive server;
const int port = 1883;//8884;
const char*myqttUser="Imesh";
const char*myqttpassword="Imesh@123";
int correct;
int i,n;
int monthDay,currentMonth,currentYear;
String currentMonthName,currentDate,formattedTime;

WiFiClient MyClient;
PubSubClient client(MyClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const long OffsetInSeconds = 19790;
 
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"Jan", "Feb", "March", "April", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {D4, D5, D6, D7}; 
byte colPins[COLS] = {D0, D1, D2, D3}; 

const int len_key = 5;
char master_key[len_key] = {'1','2','3','4','5'};
char attempt_key[len_key];
String password1; // change your password here
String input_password;
int z=0;
String myString;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() 
{
  pinMode(LOCK, OUTPUT);
  digitalWrite(LOCK, LOW);
  timeClient.begin();
  timeClient.setTimeOffset(OffsetInSeconds);
  Serial.begin(115200);
  input_password.reserve(32); // maximum input characters is 33, change if needed
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi. Please wait........");
  }
  Serial.print("Connected to WiFi :");
  Serial.println(WiFi.SSID());
  client.setServer(server, port);
  client.setCallback(MQTTcallback);
  while (!client.connected()) 
  {
    String client_id="clientId-AK9UyGk3T3";
    Serial.println("Connecting to MQTT. Please wait........");
    //if (client.connect("ESP8266"))
    if (client.connect(client_id.c_str(),myqttUser,myqttpassword))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  client.subscribe("Imesh/LOCK/Test");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  //Firebase.setString("Doorpassword", "12345");
  
  
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
  }

  Serial.print("Door Password: ");
  Serial.println(Firebase.getString("Doorpassword"));
  int count =Firebase.getInt("Count");
  if( count > 9){ 
    n=count;}
    else{
      n=10;
     // Firebase.getInt("Count");
      }
  Firebase.setInt("Count",n);
}
void MQTTcallback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Your topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String message;
  for (int i = 0; i < length; i++) 
  {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  int count =Firebase.getInt("Count");
  if (message == "0") 
  {
    digitalWrite(LOCK, HIGH);
    Serial.println("Unlock");
    client.publish("Imesh/LOCK/Test","Door Unlocked");
    if( count > 9){ 
    n=count;}
    else{
      n=10;
      }
    unsigned long epochTime = timeClient.getEpochTime();     
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    myString = String(n);
    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon+1; 
    currentMonthName = months[currentMonth-1];
    currentYear = ptm->tm_year+1900;
    formattedTime = timeClient.getFormattedTime();
    currentDate = String(currentYear) + "-" + currentMonthName + "-" + (monthDay);
    Serial.println("Door Unlocked");
    Firebase.setString("Date/Item"+myString+"/Time",formattedTime);
    Firebase.setString("Date/Item"+myString+"/Date",currentDate);
    Firebase.setString("Date/Item"+myString+"/State","ON*");
    n++;
    Firebase.setInt("Count",n);
    
  }
  else if (message == "1") 
  {
    digitalWrite(LOCK, LOW);
    Serial.println("Locked");
    client.publish("Imesh/LOCK/Test","Door Locked");
    if( count > 9){ 
    n=count;}
    else{
      n=10;
      }
    unsigned long epochTime = timeClient.getEpochTime();     
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    myString = String(n);
    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon+1; 
    currentMonthName = months[currentMonth-1];
    currentYear = ptm->tm_year+1900;
    formattedTime = timeClient.getFormattedTime();
    currentDate = String(currentYear) + "-" + currentMonthName + "-" + (monthDay);
    Serial.println("Door Locked");
    Firebase.setString("Date/Item"+myString+"/Time",formattedTime);
    Firebase.setString("Date/Item"+myString+"/Date",currentDate);
    Firebase.setString("Date/Item"+myString+"/State","OFF*");
    n++;
    Firebase.setInt("Count",n);
  }
  
  Serial.println("----------------------END----------------------");
}

void loop() 
{
  
  
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();     
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  char key = keypad.getKey();
  Serial.print(key);
    if (key){
    switch(key){
      case '#':
          password1 = Firebase.getString("Doorpassword");
          Serial.print(password1);
          delay(100); // added debounce
          if(password1 == input_password){
          //Serial.print(key);
            Serial.println("Password Correct");
            input_password = "";
            delay(1000);
            digitalWrite(LOCK, HIGH);
            myString = String(n);
            monthDay = ptm->tm_mday;
            currentMonth = ptm->tm_mon+1; 
            currentMonthName = months[currentMonth-1];
            currentYear = ptm->tm_year+1900;
            formattedTime = timeClient.getFormattedTime();
            currentDate = String(currentYear) + "-" + currentMonthName + "-" + (monthDay);
            Serial.println("Door Unlocked");
            Firebase.setString("Date/Item"+myString+"/Time",formattedTime);
            Firebase.setString("Date/Item"+myString+"/Date",currentDate);
            Firebase.setString("Date/Item"+myString+"/State","ON");
            n++;
            Firebase.setInt("Count",n);
          }
          else
          {
            Serial.println("Incorrect Password");
            input_password = "";
            Serial.println("Insert Correct Password");
          }
          
       break;
      case '*':
        delay(100); // added debounce
        Serial.println("Door is Locked");
        digitalWrite(LOCK, LOW);
        myString = String(n);
        monthDay = ptm->tm_mday;
        currentMonth = ptm->tm_mon+1; 
        currentMonthName = months[currentMonth-1];
        currentYear = ptm->tm_year+1900;
        formattedTime = timeClient.getFormattedTime();
        currentDate = String(currentYear) + "-" + currentMonthName + "-" + (monthDay);
        Firebase.setString("Date/Item"+myString+"/Time",formattedTime);
        Firebase.setString("Date/Item"+myString+"/Date",currentDate);
        Firebase.setString("Date/Item"+myString+"/State","OFF");
        input_password = "";
        n++;
        Firebase.setInt("Count",n);
        break;
      default:
        input_password += key; 
      }
        
  }//else{
     
  client.loop();
  //  }
}
