#include <AESLib.h>

#include <SPI.h>
#include <Ethernet.h>

#include "RestClient.h"

int test_delay = 1000; //so we don't spam the API
boolean describe_tests = true;

//reusable test variables
char* post_body = "POSTDATA";
String response;

IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);

RestClient client = RestClient("arduino-http-lib-test.herokuapp.com");
//RestClient client = RestClient("192.168.1.50",5000);

void setup() {
  // put your setup code here, to run once:
  client.dhcp();
  aesTest();
  serverSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  POST_tests();
  createServer();
}

void aesTest() {
  Serial.begin(57600);
  uint8_t key[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  char data[] = "0123456789012345"; //16 chars == 16 bytes
  //char data[] = "Das ist ein Test";
  aes128_enc_single(key, data);
  Serial.print("encrypted:");
  Serial.println(data);
  aes128_dec_single(key, data);
  Serial.print("decrypted:");
  Serial.println(data);
}

void serverSetup(){
  // Pin 4 used for sdcard, prevent hang up
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
  };
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void test_status(int statusCode){
   delay(test_delay);
   if(statusCode == 200){
    Serial.print("TEST RESULT: ok (");
    Serial.print(statusCode);
    Serial.println(")");
   }else{
    Serial.print("TEST RESULT: fail (");
    Serial.print(statusCode);
    Serial.println(")");
   }
}

void describe(char * description){
  if(describe_tests) Serial.println(description);
}

void test_response(){
  //Serial.println(response);
  if(response == "OK"){
   Serial.println("TEST RESULT: ok (response body)");
  }else{
   Serial.println("TEST RESULT: fail (response body = " + response + ")");
  }
  response = "";
}

void POST_tests(){
    // POST TESTS
  describe("Test POST with path and body");
  test_status(client.post("/data", post_body));

  describe("Test POST with path and body and response");
  test_status(client.post("/data", post_body, &response));
  test_response();

  describe("Test POST with path and body and header");
  client.setHeader("X-Test-Header: true");
  test_status(client.post("/data-header", post_body));

  describe("Test POST with path and body and header and response");
  client.setHeader("X-Test-Header: true");
  test_status(client.post("/data-header", post_body, &response));
  test_response();

  describe("Test POST with 2 headers and response");
  client.setHeader("X-Test-Header1: one");
  client.setHeader("X-Test-Header2: two");
  test_status(client.post("/data-headers", post_body, &response));
  test_response();
}

void createServer(){
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
