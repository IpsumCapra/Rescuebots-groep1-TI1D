#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


#define FORWARD 32
#define BACKWARD 33
#define RIGHT 34
#define LEFT 35
#define HALT 36


/* Put your SSID & Password */
const char* ssid = "Tesla IoT";  // Enter SSID here
const char* password = "fsL6HgjN";  //Enter Password here

ESP8266WebServer server(666);

uint8_t left_Forward = D2;
uint8_t left_Backward = D3;
uint8_t right_Forward = D4;
uint8_t right_Backward = D5;


bool lightState = LOW;

int dir;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(left_Forward, OUTPUT);
  pinMode(left_Backward, OUTPUT);
  pinMode(right_Forward, OUTPUT);
  pinMode(right_Backward, OUTPUT);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname("porterbot");
  
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
  
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/light", handle_light);
  server.on("/up", handle_up);
  server.on("/left", handle_left);
  server.on("/right", handle_right);
  server.on("/down", handle_down);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  motorLogic();
}

void handle_OnConnect() {
  Serial.println("halting");
  dir = HALT;
  server.send(200, "text/html", SendHTML()); 
}

void handle_light() {
  lightState = !lightState;
  digitalWrite(LED_BUILTIN, lightState);
  Serial.println("toggling lights.");
  server.send(200, "text/html", SendHTML()); 
}

void handle_up() {
  Serial.println("Going forward.");
  dir = FORWARD;
  server.send(200, "text/html", SendHTML()); 
}

void handle_left() {
  Serial.println("Going left.");
  dir = LEFT;
  server.send(200, "text/html", SendHTML()); 
}

void handle_right() {
  Serial.println("Going right.");
  dir = RIGHT;
  server.send(200, "text/html", SendHTML()); 
}

void handle_down() {
  Serial.println("Going backwards.");
  dir = BACKWARD;
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="</head>\n";
  ptr +="<style>\n";
  ptr +="#controller {\n";
  ptr +="float: left;\n";
  ptr +="overflow: auto;\n";
  ptr +="}\n";
  ptr +="#data {\n";
  ptr +="padding: 10px;\n";
  ptr +="float: left;\n";
  ptr +="overflow: auto;\n";
  ptr +="}       \n";
  ptr +="#controller>.dpad {\n";
  ptr +="display: grid;\n";
  ptr +="width: auto;\n";
  ptr +="height: auto;\n";
  ptr +="padding: 10px;\n";
  ptr +="grid-gap: 10px;\n";
  ptr +="grid-template-areas: \". accelerate .\"\n";
  ptr +="\"left honk right\"\n";
  ptr +="\". reverse .\";\n";
  ptr +="}\n";
  ptr +="#controller>.dpad>button[key^=\"KEY_UP\"] {\n";
  ptr +="grid-area: accelerate;\n";
  ptr +="}\n";
  ptr +="#controller>.dpad>button[key^=\"KEY_LEFT\"] {\n";
  ptr +="grid-area: left;\n";
  ptr +="}\n";
  ptr +="#controller>.dpad>button[key^=\"KEY_RIGHT\"] {\n";
  ptr +="grid-area: right; \n";
  ptr +="}\n";
  ptr +="#controller>.dpad>button[key^=\"KEY_DOWN\"] {\n";
  ptr +="grid-area: reverse;\n";
  ptr +="}\n";
  ptr +="#controller>.dpad>button[key^=\"HONK\"] {\n";
  ptr +="grid-area: honk;\n";
  ptr +="}\n";
  ptr +="</style>\n";
  ptr +="<script>\n";
  ptr +="function func(dir) {\n";
  ptr +="location.href = dir;\n";
  ptr +="}\n";
  ptr +="</script>\n";
  ptr +="<body>\n";
  ptr +="<h1><i>Porter-Bot Controller V1.0</i></h1>\n";
  ptr +="<div id=\"controller\">\n";
  ptr +="<div class=\"dpad\">\n";
  ptr +="<button key=\"KEY_UP\" onmousedown=\"func('/up')\" onmouseup=\"func('/')\">ACCELERATE</button>\n";                                                         
  ptr +="<button key=\"KEY_LEFT\" onmousedown=\"func('/left')\" onmouseup=\"func('/')\">LEFT</button>\n";
  ptr +="<button key=\"KEY_RIGHT\" onmousedown=\"func('/right')\" onmouseup=\"func('/')\">RIGHT</button>\n";
  ptr +="<button key=\"KEY_DOWN\" onmousedown=\"func('/down')\" onmouseup=\"func('/')\">REVERSE</button>\n";
  ptr +="<button key=\"HONK\" onmousedown=\"func('/light')\" onmouseup=\"func('/')\">HONK</button>\n";
  ptr +="</div>\n";     
  ptr +="</div>\n";  
  ptr +="<button onmousedown=\"func('/light')\" onmouseup=\"func('/')\">lights</button>\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void motorLogic()
{
  switch(dir)
  {
    case FORWARD:
    motor_Forwards();
    break;
    case BACKWARD:
    mtor_Backwards();
    break;
    case RIGHT:
    motor_Right();
    break;
    case LEFT:
    motor_Left();
    break;
    case HALT:
    breaking();
    break;
    default:
    digitalWrite(left_Forward , LOW);
    digitalWrite(left_Backward , LOW);
   digitalWrite(right_Forward , LOW);
    digitalWrite(right_Backward , LOW);
    break;
    }
}

//Makes the motor go forward
void motor_Forwards()
{
  digitalWrite(left_Forward , HIGH);
  digitalWrite(left_Backward , LOW);
  digitalWrite(right_Forward , HIGH);
  digitalWrite(right_Backward , LOW);
}
//Makes the motor go backwards
void mtor_Backwards()
{
  digitalWrite(left_Forward , LOW);
  digitalWrite(left_Backward , HIGH);
  digitalWrite(right_Forward , LOW);
  digitalWrite(right_Backward , HIGH);
}
//Makes the motor turn right
void motor_Right()
{
  digitalWrite(left_Forward , HIGH);
  digitalWrite(left_Backward , LOW);
  digitalWrite(right_Forward , LOW);
  digitalWrite(right_Backward , HIGH);
}

//Makes the motor turn left
void motor_Left()
{
  digitalWrite(left_Forward , LOW);
  digitalWrite(left_Backward , HIGH);
  digitalWrite(right_Forward , HIGH);
  digitalWrite(right_Backward , LOW);
}

//Makes the motor stop
void breaking()
{
  digitalWrite(left_Forward , LOW);
  digitalWrite(left_Backward , LOW);
  digitalWrite(right_Forward , LOW);
  digitalWrite(right_Backward , LOW);
}
