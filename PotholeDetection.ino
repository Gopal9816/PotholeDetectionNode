#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include<PubSubClient.h>

//Credentials for WiFi and mqtt server
const char* ssid = "";//"";
const char* password = "";//"";
const char* mqtt_server = "iot.eclipse.org";
int port = 1883;

//led pin

//Client object
WiFiClient espClient;
PubSubClient client(espClient);

//MQTT Client
char* clientID = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  
  //Connecting to WiFi
  setupWiFi();
  
  //Connecting to mqtt server
  setupServer();
  
  //Ultrasonic
  int triggerPin = D2;
  int echoPin = D3;
}

void setupWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  Serial.print("Successfully Connected to ");
  Serial.println(ssid);
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
void setupServer(){
  client.setServer(mqtt_server,port);
  while(!client.connected()){
    Serial.println("Attempting to connect to MQTT server...");
    if(client.connect(clientID)){
      Serial.println("Connected to server successfully");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  client.loop();
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();

  long duration, inches, cm;

  pinMode(triggerPin ,OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(triggerPin, LOW);

  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);

  if(cm > 50){
      //mark GPS coordinates of pothole
      float lat, longt;
      getLocation(&lat,&longt);
      JSONencoder["device"] = "ESP32";
      JSONencoder["Latitude"] = lat;
      JSONencoder["Longitude"] = longt;
      char JSONmessageBuffer[100];
      JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      Serial.println("Publish message: Location ");
      client.publish("data_channel", JSONmessageBuffer);
      delay(5000);
  }
  
  delay(2000);
 
}
long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
void getLocation(float *lat, float *longt){
  //We are looking to record location using a GPS module however we  
  //didn't have one during the course of development, Hence we are
  //sending a dummy data
  *lat = 9.937921;
  *longt = 76.261565;
}

