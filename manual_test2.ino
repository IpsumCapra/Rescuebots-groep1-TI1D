#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define FORWARD 32
#define BACKWARD 33
#define RIGHT 35
#define LEFT 34
#define HALT 36

#define IR_PIN_LEFT D1
#define IR_PIN_RIGHT D2
int ir_left;
int ir_right;


/* Put your SSID & Password */
const char* ssid = "Tesla IoT";  // Enter SSID here
const char* password = "fsL6HgjN";  //Enter Password here

ESP8266WebServer server(666);

uint8_t buzzer = D0;

uint8_t left_Forward = D5;
uint8_t left_Backward = D6;
uint8_t right_Forward = D7;
uint8_t right_Backward = D8;


bool lightState = LOW;
bool autoToggle = false;

bool blinker;
bool beep = false;

int dir;
int evadeDir;
int lastEvadeDir;
int evades;

bool evade;
bool revert;

unsigned long startEvade;
unsigned long blinkerMillis;
unsigned long evadeCooldownMillis;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(left_Forward, OUTPUT);
  pinMode(left_Backward, OUTPUT);
  pinMode(right_Forward, OUTPUT);
  pinMode(right_Backward, OUTPUT);
  pinMode(IR_PIN_LEFT, INPUT);
  pinMode(IR_PIN_RIGHT, INPUT);
  pinMode(buzzer, OUTPUT);

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
  server.on("/buzzer", handle_buzzer);
  server.on("/toggle", handle_toggle);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  motorLogic();

  if (blinker) {
    if (millis() - blinkerMillis >= 500) {
      blinkerMillis = millis();
      if (beep) {
        tone(buzzer,1000);
      } else {
        noTone(buzzer);
      }
      beep = !beep;
    }
  }
  
  if (autoToggle) {
    if (dir != FORWARD) {
      dir = FORWARD;
    }
    
    ir_left = digitalRead(IR_PIN_LEFT);
    ir_right = digitalRead(IR_PIN_RIGHT);
    
    if(ir_left == HIGH){
      prepEvade(RIGHT);
    }
    if(ir_right == HIGH){
      prepEvade(LEFT);
    }

    if (evade && !revert) {
      if (millis() - startEvade < 500) {
        dir = BACKWARD;
      } else if (millis() - startEvade < 2000) {
        dir = evadeDir;
      } else {
        evade = false;
        evadeCooldownMillis = millis();
        if (evadeDir != lastEvadeDir) {
          evades++;
        }
        lastEvadeDir = evadeDir;
      }
    }

    if (millis() - evadeCooldownMillis >= 15000) {
      evades = 0;
    }

    if (evades == 3) {
      evadeCooldownMillis = millis();
      revert = true;
      evades = 0;
      /*
      if (evadeDir == RIGHT) {
        evadeDir = LEFT;
      } else {
        evadeDir = RIGHT;
      }
      */
    }

    if (revert) {
      if (millis() - evadeCooldownMillis < 500) {
        dir = BACKWARD;
      } else if (millis() - evadeCooldownMillis < 4500) {
        dir = evadeDir;
      } else {
        revert = false;
      }
    }
  }
}

void prepEvade(int dir) {
  if (!evade) {
    evade = true;
    startEvade = millis();
    evadeDir = dir;
  }
}

void handle_OnConnect() {
  Serial.println("halting");
  dir = HALT;
  noTone(buzzer);
  beep = true;
  blinker = false;
  server.send(200, "text/html", SendHTML()); 
}

void handle_toggle() {
  Serial.println("Toggling automatic mode.");
  tone(buzzer, 400);
  delay(100);
  noTone(buzzer);
  delay(100);
  tone(buzzer, 400);
  delay(300);
  noTone(buzzer);
  autoToggle = !autoToggle;
  dir = HALT;
  server.send(200, "text/html", SendHTML());  
}

void handle_buzzer() {
  tone(buzzer, 400);
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
  blinker = true;
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  ptr +="<title>Porterbot Control</title>\n";
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
  ptr +="-webkit-user-select:none;\n";
  ptr +="display: grid;\n";
  ptr +="width: auto;\n";
  ptr +="height: 200px;\n";
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
  ptr +="var xmlHttp=createXmlHttpObject();\n";
  ptr +="function createXmlHttpObject(){\n";
  ptr +=" if(window.XMLHttpRequest){\n";
  ptr +="    xmlHttp=new XMLHttpRequest();\n";
  ptr +=" }else{\n";
  ptr +="    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  ptr +=" }\n";
  ptr +=" return xmlHttp;\n";
  ptr +="}\n";
  ptr +="function func(dir) {\n";
  ptr +="    xmlHttp.open('SET',dir,true);\n";
  ptr +="    xmlHttp.send(null);\n";
  ptr +="}\n";
  ptr +="</script>\n";
  ptr +="<body>\n";
  ptr +="<h1><i>Porter-Bot Controller V2.1</i></h1>\n";
  ptr +="<div id=\"controller\">\n";
  ptr +="<div class=\"dpad\">\n";
  ptr +="<button key=\"KEY_UP\" ontouchstart=\"func('up')\" ontouchend=\"func('')\" onmousedown=\"func('up')\" onmouseup=\"func('')\">ACCELERATE</button>\n";                                                         
  ptr +="<button key=\"KEY_LEFT\" ontouchstart=\"func('left')\" ontouchend=\"func('')\" onmousedown=\"func('left')\" onmouseup=\"func('')\">LEFT</button>\n";
  ptr +="<button key=\"KEY_RIGHT\" ontouchstart=\"func('right')\" ontouchend=\"func('')\" onmousedown=\"func('right')\" onmouseup=\"func('')\">RIGHT</button>\n";
  ptr +="<button key=\"KEY_DOWN\" ontouchstart=\"func('down')\" ontouchend=\"func('')\" onmousedown=\"func('down')\" onmouseup=\"func('')\">REVERSE</button>\n";
  ptr +="<button key=\"HONK\" ontouchstart=\"func('buzzer')\" ontouchend=\"func('')\" onmousedown=\"func('buzzer')\" onmouseup=\"func('')\">HONK</button>\n";
  ptr +="</div>\n";     
  ptr +="</div>\n";  
  ptr +="<button ontouchstart=\"func('light')\" onmousedown=\"func('light')\">lights</button>\n";
  ptr +="<button ontouchstart=\"func('toggle')\" onmousedown=\"func('toggle')\">toggle auto/manual</button>\n";

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
