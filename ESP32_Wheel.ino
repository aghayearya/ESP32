#include <WiFi.h>
#include <WiFiUdp.h>
 
// WiFi network name and password:
const char * networkName = "Arya";
const char * networkPswd = "09380936";
//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char * udpAddress = "172.20.10.11";
const int udpPort = 3333;
//Are we currently connected?
boolean connected = false;
//The udp library class
WiFiUDP udp;
char packet[255];
int wheelDir = -1;
boolean sendWifi = false;
 
const int debugLED = LED_BUILTIN;
 
// Variables For Wheel Support
const int hallSensor1 = 19;
const int hallSensor2 = 23;
 
boolean states1[ 2 ][ 8 ];
int steps[8];
 
boolean compare1 = false;
boolean compare2 = false;
boolean compare3 = false;
boolean compare4 = false;
 
String wheel_direction = "undefined";
 
const int trace_der1[] = { 3, 1, 3, 0 };
const int trace_der2[] = { 1, 3, 0, 3 };
const int trace_der3[] = { 3, 0, 3, 1 };
const int trace_der4[] = { 0, 3, 1, 3 };
 
const int trace_izq1[] = { 3, 1, 2, 1 };
const int trace_izq2[] = { 1, 2, 1, 3 };
const int trace_izq3[] = { 2, 1, 3, 1 };
const int trace_izq4[] = { 1, 3, 1, 2 };
 
int traceSize = 4;
boolean sensorupdated = false;
int wheelCount = 0;
 
// Flags
boolean enableWheelEvent = true;
boolean enableStandaloneDemo = false;
 
// User Config
uint8_t wheelEventResolution = 1; // 1 = Each step is reportet, 2 = each 2nd Step is reported, etc..
uint8_t wheelTmp = 1;


 
 
void setup() {
 // Start the serial port
 Serial.begin(115200);
 
 
 //Connect to the WiFi network
 connectToWiFi(networkName, networkPswd);
 delay(5000);
 
 String Data = "Wifi Initialized";
 
 //Data += "ElapsedTime:" + String(millis()/1000) + ",";
 Data.toCharArray( packet , Data.length() );
 //Send a packet
 udp.beginPacket(udpAddress,udpPort);
 udp.printf(packet);
 udp.endPacket();


 
 // Init Wheel
 pinMode(hallSensor1, INPUT);
 //digitalWrite( hallSensor1, HIGH ); //To enable an internal 20K pullup resistor (see http://arduino.cc/en/Reference/DigitalWrite)
 attachInterrupt( hallSensor1, &rpm_interrupt1, CHANGE );
 pinMode(hallSensor2, INPUT );
 //digitalWrite( hallSensor2, HIGH ); //To enable an internal 20K pullup resistor (see http://arduino.cc/en/Reference/DigitalWrite)
 
}
 
void loop() {
 refreshWheelData();
 
 
 // Debug
 //Serial.println(String("Hallsensor1: ") + digitalRead( hallSensor1 ) + String(" Hallsensor2: ") + digitalRead( hallSensor2 ));
 
 if(sendWifi){
    //Data += "ElapsedTime:" + String(millis()/1000) + ",";
    String Data = "";
    Data += (String)wheelDir ;
    Serial.println(Data);
 
    Data.toCharArray( packet , Data.length() + 1 );
    //Send a packet
    udp.beginPacket(udpAddress,udpPort);
    udp.printf(packet);
    udp.endPacket();
 
    
    sendWifi = false;
  }
 
}
 
void sendSensorData() {
 
}
 
boolean testState = false;
void toggleDebugLed() {
 if (testState) {
 digitalWrite(debugLED, HIGH);
 testState = false;
 } else {
 digitalWrite(debugLED, LOW);
 testState = true;
 }
}
 
void refreshWheelData() {
 if (sensorupdated) {
 detachInterrupt( hallSensor1 );
 for ( int col = 0; col <= 3; col++ ) {
 compare1 = compareArrays( trace_der1, steps, traceSize );
 compare2 = compareArrays( trace_der2, steps, traceSize );
 compare3 = compareArrays( trace_der3, steps, traceSize );
 compare4 = compareArrays( trace_der4, steps, traceSize );
 // compare4 = ((steps[ col +0 ] == trace_der4[ 0 ]) && (steps[ col +1 ] == trace_der4[ 1 ]) && (steps[ col +2 ] == trace_der4[ 2 ]) && (steps[ col +3 ] == trace_der4[ 3 ]));
 
 if ( compare1 || compare2 || compare3 || compare4 ) {
 wheel_direction = "right";
 // Do something here for right
 //allLEDOff();
 uint8_t data[2];
 data[0] = 21;
 data[1] = 1;
 //Tx16Request tx1 = Tx16Request(0x1234, ACK_OPTION, data, 2, DEFAULT_FRAME_ID);
 toggleDebugLed();
 //xbee.send(tx1);
 Serial.println("right");
 
 wheelDir = 1;
 sendWifi = true;
 
 
 //measuredDistance = measuredDistance-cpm;
 
 
 
 break;
 } else {
 compare1 = compareArrays( trace_izq1, steps, traceSize );
 compare2 = compareArrays( trace_izq2, steps, traceSize );
 compare3 = compareArrays( trace_izq3, steps, traceSize );
 compare4 = compareArrays( trace_izq4, steps, traceSize );
 // compare4 = ((steps[ col +0 ] == trace_izq4[ 0 ]) && (steps[ col +1 ] == trace_izq4[ 1 ]) && (steps[ col +2 ] == trace_izq4[ 2 ]) && (steps[ col +3 ] == trace_izq4[ 3 ]));
 
 if ( compare1 || compare2 || compare3 || compare4 ) {
 wheel_direction = "left";
 // Do something for left direction
 uint8_t data[2];
 data[0] = 21;
 data[1] = 2;
 //Tx16Request tx1 = Tx16Request(0x1234, ACK_OPTION, data, 2, DEFAULT_FRAME_ID);
 toggleDebugLed();
 //xbee.send(tx1);
 Serial.println("left");
 
 
 wheelDir = 0;
 sendWifi = true;
 
 //measuredDistance = measuredDistance+cpm;
 
 
 break;
 } else {
 wheel_direction = "undefined";
 // Do something for undefined
 }
 }
 }
 //Serial.println( wheel_direction );
 attachInterrupt( hallSensor1, &rpm_interrupt1, CHANGE );
 }
 sensorupdated = false;
}
 
// Wheel
boolean compareArrays( const int *traceArray, int *readArray, int sizeArray )
{
 boolean areEqual = true; //We start on the assumption that both arrays are equal
 int matches = 0;
 int index = 0;
 
 while ( ( areEqual ) && ( index < sizeArray ) )
 {
 if ( traceArray[ index ] == readArray[ index ] )
 {
 index++;
 }
 else
 {
 areEqual = false;
 }
 }
 
 return areEqual;
 
}
 
void rpm_interrupt1()
{
 
 states1[ 0 ][ wheelCount ] = digitalRead( hallSensor1 ); //To determine direction
 states1[ 1 ][ wheelCount ] = digitalRead( hallSensor2 );
 steps[ wheelCount ] = int (states1[ 0 ][ wheelCount ]) * 2 + int( states1[ 1 ][ wheelCount ] ) * 1;
 wheelCount++;
 if ( wheelCount > traceSize - 1 )
 {
 wheelCount = 0;
 }
 
 sensorupdated = true;
}
 
void connectToWiFi(const char * ssid, const char * pwd){
 Serial.println("Connecting to WiFi network: " + String(ssid));
 // delete old config
 WiFi.disconnect(true);
 //register event handler
 WiFi.onEvent(WiFiEvent);
 
 //Initiate connection
 WiFi.begin(ssid, pwd);
 Serial.println("Waiting for WIFI connection...");
}
//wifi event handler
void WiFiEvent(WiFiEvent_t event){
 switch(event) {
 case ARDUINO_EVENT_WIFI_STA_GOT_IP:
 //When connected set
 Serial.print("WiFi connected! IP address: ");
 Serial.println(WiFi.localIP()); 
 //initializes the UDP state
 //This initializes the transfer buffer
 udp.begin(WiFi.localIP(),udpPort);
 connected = true;
 break;
 case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
 Serial.println("WiFi lost connection");
 connected = false;
 break;
 default: break;
 }
}