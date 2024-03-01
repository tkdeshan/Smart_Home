/*********
  Kavinda Deshan
  Complete project details at http://randomnerdtutorials.com  
*********/
#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// Network credentials
const char* ssid = "Dialog 4G 700";
const char* password = "46F8b9C1";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String userPassword = "";
String passwordState = "Wrong";
String bulbInsideState = "OFF";
String bulbOutsideState = "OFF";
String switchState = "OFF";
String gateState = "CLOSE";
String ldrState = "NOT DETECTED";

// Assign output variables to GPIO pins
const int bulbInside = 12;
const int bulbOutside = 27;
const int switchMotor = 14;
const int gate = 13;
const int buzzer = 26;
const int ldr = A0;

// Servo motor init
Servo gateServo;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  // Initialize the output variables as outputs
  pinMode(bulbInside, OUTPUT);
  pinMode(bulbOutside, OUTPUT);
  pinMode(switchMotor, OUTPUT);
  pinMode(ldr, INPUT);
  pinMode(buzzer, OUTPUT);
  gateServo.attach(gate);

  // Set outputs to LOW
  digitalWrite(bulbInside, LOW);
  digitalWrite(bulbOutside, LOW);
  digitalWrite(switchMotor, LOW);
  digitalWrite(buzzer, LOW);
  gateServo.write(0);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  int rawDataLdr = analogRead(ldr);

  // Check left LDR status
  if (rawDataLdr < 3000) {
    ldrState = "OBJECT DETECTED";
    digitalWrite(buzzer, HIGH);
  } else {
    ldrState = "NOT DETECTED";
    digitalWrite(buzzer, LOW);
  }

  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");                                             // print a message out in the serial port
    String currentLine = "";                                                   // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            // client.println("HTTP/1.1 200 OK");
            // client.println("Content-type:text/html");
            // client.println("Connection: close");
            // client.println();

            // Handle the password requests and confirm password
            if (header.indexOf("GET /auth?pwd=1234") >= 0) {
              userPassword = "1234";
              passwordState = "Correct";
              Serial.println(userPassword);
            }

            // Handle the outputs according to the requests
            if (header.indexOf("GET /12/on") >= 0) {
              Serial.println("Bulb Inside on");
              bulbInsideState = "ON";
              digitalWrite(bulbInside, HIGH);
            } else if (header.indexOf("GET /12/off") >= 0) {
              Serial.println("Bulb Inside off");
              bulbInsideState = "OFF";
              digitalWrite(bulbInside, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("Bulb Outside on");
              bulbOutsideState = "ON";
              digitalWrite(bulbOutside, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("Bulb Outside off");
              bulbOutsideState = "OFF";
              digitalWrite(bulbOutside, LOW);
            } else if (header.indexOf("GET /14/on") >= 0) {
              Serial.println("Switch on");
              switchState = "ON";
              digitalWrite(switchMotor, HIGH);
            } else if (header.indexOf("GET /14/off") >= 0) {
              Serial.println("Switch off");
              switchState = "OFF";
              digitalWrite(switchMotor, LOW);
            } else if (header.indexOf("GET /13/open") >= 0) {
              Serial.println("Gate Open");
              if (userPassword == "1234") {
                gateState = "OPEN";
                gateServo.write(100);
              }
            } else if (header.indexOf("GET /13/close") >= 0) {
              Serial.println("Gate close");
              gateState = "CLOSE";
              gateServo.write(0);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".heading { background-color: #ebe534; padding: 8px 0px;}");
            client.println(".password_field { background-color: #75a4f0; display: flex; flex-direction: column; justify-content: center; }");
            client.println(".input_container { display: flex; flex-direction: row; justify-content: center; }");
            client.println("input { outline: none; padding: 11px 4px;}");
            client.println(".btn_submit { background-color: #4CAF50; border: none; color: white; padding: 9px 8px; text-decoration: none; font-size: 20px; cursor: pointer; }");
            client.println(".container { width : 100%; background-color: #32a8a6; display: flex; justify-content: center;}");
            client.println(".container_color { background-color: #75a4f0; }");
            client.println(".control_row { width : 190px; display: flex; flex-direction: row; justify-content: space-between; }");
            client.println(".control_col { width : 190px; display: flex; flex-direction: column;  justify-content: center; }");
            client.println(".state_container { background-color: #75a4f0; display: flex; flex-direction: column;  justify-content: center; }");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 9px 12px; text-decoration: none; font-size: 20px; cursor: pointer; min-width: 80px; margin-left: 10px}");
            client.println(".button2 { background-color: #555555; }</style></head>");

            // Web page start
            client.println("<body>");

            // Heading
            client.println("<div class=\"heading\">");
            client.println("<h2>Smart Home Dashboard</h2>");
            client.println("</div>");

            // JavaScript function for automatic page reload
            client.println("<script>");
            client.println("function reloadPage() {");
            client.println("  setTimeout(function(){location.reload();}, 20000);");  // Reload page every 2 minute
            client.println("}");
            client.println("reloadPage();");  // Call the function when the page loads
            client.println("</script>");

            // Display Securty Status
            client.println("<div class=\"state_container\">");
            client.println("<p>Security Status - " + ldrState + "</p>");
            client.println("<p>Fire Status - NOT DETECTED</p>");
            client.println("</div>");

            // Display camera
            client.println("<div class=\"container\">");
            client.println("<p>Camera</p>");
            client.println("</div>");

            // Password input field
            client.println("<div class=\"password_field\">");
            client.println("<div class=\"input_container\">");
            client.println("<p><input type=\"text\" id=\"passwordInput\" placeholder=\"Enter password\"></p>");
            client.println("<p><a href=\"/auth\"><button class=\"btn_submit\" onclick=\"submitPassword()\">Submit</button></a></p>");
            client.println("</div>");
            client.println("<p>" + passwordState + " Password</p>");
            client.println("</div>");

            // Scripts for get the password from client
            client.println("<script>");
            client.println("function submitPassword() {");
            client.println("  var enteredPassword = document.getElementById('passwordInput').value;");
            client.println("  window.location.href = '/auth?pwd=' + enteredPassword;");
            client.println("alert(\"processing...\");");
            client.println("}");
            client.println("</script>");

            // Display current state, and OPEN/OCLOSE buttons for gate
            client.println("<div class=\"container\">");
            client.println("<div class=\"control_row\">");
            client.println("<p>Gate State " + gateState + "</p>");

            if (gateState == "CLOSE") {
              client.println("<p><a href=\"/13/open\"><button class=\"button\">OPEN</button></a></p>");
            } else {
              client.println("<p><a href=\"/13/close\"><button class=\"button button2\">CLOSE</button></a></p>");
            }

            client.println("</div>");
            client.println("</div>");

            // Display current state, and ON/OFF buttons for Bulb Inside
            client.println("<div class=\"container container_color\">");
            client.println("<div class=\"control_row\">");
            client.println("<p>Bulb Inside " + bulbInsideState + "</p>");

            if (bulbInsideState == "OFF") {
              client.println("<p><a href=\"/12/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/12/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</div>");
            client.println("</div>");

            // Display current state, and ON/OFF buttons for Bulb Outside
            client.println("<div class=\"container\">");
            client.println("<div class=\"control_row\">");
            client.println("<p>Bulb Outside " + bulbOutsideState + "</p>");

            if (bulbOutsideState == "OFF") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</div>");
            client.println("</div>");

            // Display current state, and ON/OFF buttons for switch
            client.println("<div class=\"container container_color\">");
            client.println("<div class=\"control_row\">");
            client.println("<p>Switch State " + switchState + "</p>");

            if (switchState == "OFF") {
              client.println("<p><a href=\"/14/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/14/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</div>");
            client.println("</div>");

            // The HTTP response ends with another blank line
            // client.println();
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // End of html page
  client.println("</body></html>");
}