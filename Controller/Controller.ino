#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>


// optional you can enable/disable serial status-information
#define serial true

// Pin definitions
#define PinRed D2
#define PinGreen D3
#define PinBlue D1


// some global variables:

// serial output of the ip-adresse
unsigned long last_ip_poll;
const int delay_between_ip_poll = 5000;

// get the status, if the animation is running
bool animation_running;

// the current position of the animation
int animState;

// the colors of the animation
int animColors[3];

// delays for the animation
unsigned long lastAnimPoll;
int delayAnim = 100;


// set up the access point
const char* ssid = "LED-Streifen";
const char* password = "12345678";


// set up the dns server
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
const char* dns_adresse = "*";


// initialise the server
ESP8266WebServer server(80);


// ---------------------------- The setup routine ----------------------------
void setup( void ) {
  // start services:

  // display the information that the access point is going to be activated
  if (serial) {
    Serial.begin(115200);
    Serial.println("starting access point...");
  }

  // open the wifi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  //      Optional configuration for channel and hidden ssid and ip-adresse of the access-point
  //      the function returns true / false if it succeed. maybe if the connesctioncolud not been opened, ask the user what to do via serial monitor

  // display the ip adresse of the access point
  if (serial) {
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Access Point IP address: ");
    Serial.println(myIP);
  }


  // init server handles
  server.on("/", handleRoot);
  server.on("/power", handlePower);
  server.on("/animation", handleAnimation);

  server.onNotFound([]() {
    server.send(404, "text/html", "");
  });

  // open the server
  server.begin();

  // notify the user that the http server started
  if (serial) {
    Serial.println("HTTP server started");
  }


  // start dns server
  dnsServer.start(DNS_PORT, dns_adresse, apIP);


  // initialisations:

  // initialise the variable to display the ip adresse
  if (serial) {
    last_ip_poll = millis();
  }

  // initialisation-state of the animation
  animState = 0;
  animation_running = true;
  lastAnimPoll = millis();

  // init animation colors
  animColors[0] = 255;
  animColors[1] = 0;
  animColors[2] = 0;
}


// ---------------------------- The loop routine ----------------------------
void loop( void ) {
  // make the dns server run
  dnsServer.processNextRequest();


  // make the server run
  server.handleClient();

  // display the ip adresse on the serial monitor
  if (serial) {
    if ((millis() - last_ip_poll) > delay_between_ip_poll) {
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("IP address: ");
      Serial.println(myIP);
      last_ip_poll = millis();
    }
  }

  // if the animation is activated, make it run
  if (animation_running && (millis() - lastAnimPoll) > delayAnim) {
    lastAnimPoll = millis();
    animation();
    analogWrite(PinRed, animColors[0] * 4);
    analogWrite(PinGreen, animColors[1] * 4);
    analogWrite(PinBlue, animColors[2] * 4);
  }
}


// ---------------------------- The server handles ----------------------------

// root handle
void handleRoot( void ) {
  server.send(200, "text/html", "<!DOCTYPE html><html><head><title>LED-Streifen</title><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body,main{display:flex;align-content:center}header,main{line-height:1.5}body,html,section{background-color:#fff}#color-label,body,html,section p{padding:0;margin:0}body,html{width:100%;height:100%;border:none;font-size:100%;font-family:sans-serif}body{flex-direction:column;align-items:center;justify-content:space-around}header{display:block;min-width:100vw;margin:0;padding:5% 5% 2%;color:#fff;background-color:#08c;border-bottom:solid 1px #0d5f92;font-size:3rem;text-align:center;flex:0 0 auto}main{flex-direction:column;align-items:center;justify-content:space-around;font-size:1.8rem;flex:1 1 auto}input[type=button]{display:block;font-size:inherit;line-height:inherit;background-color:#1e88e5;color:#fff;border:none;cursor:pointer;margin:0;padding:.5rem;min-width:380px}input[type=button]:active,input[type=button]:focus,input[type=button]:hover{background-color:#1f71e4}section{border:1px solid #ccc;padding:5px}section p{font-size:1.5rem;line-height:1;display:inline}canvas:hover{cursor:crosshair}#color-label{cursor:default;height:1.5rem;background-color:#000}</style><script>window.addEventListener('load',function(){var t=document.getElementById('color-block'),e=t.getContext('2d'),o=t.width,a=t.height,d=document.getElementById('color-strip'),r=d.getContext('2d'),n=d.width,l=d.height,i=document.getElementById('color-label'),c=0,g=0,f=!1,b='rgba(255,0,0,1)';e.rect(0,0,o,a),u(),r.rect(0,0,n,l);var s=r.createLinearGradient(0,0,0,a);function u(){e.fillStyle=b,e.fillRect(0,0,o,a);var t=r.createLinearGradient(0,0,o,0);t.addColorStop(0,'rgba(255,255,255,1)'),t.addColorStop(1,'rgba(255,255,255,0)'),e.fillStyle=t,e.fillRect(0,0,o,a);var d=r.createLinearGradient(0,0,0,a);d.addColorStop(0,'rgba(0,0,0,0)'),d.addColorStop(1,'rgba(0,0,0,1)'),e.fillStyle=d,e.fillRect(0,0,o,a)}function p(t){c=t.offsetX,g=t.offsetY;var o=e.getImageData(c,g,1,1).data;b='rgba('+o[0]+','+o[1]+','+o[2]+',1)',i.style.backgroundColor=b;var a=i.style.backgroundColor.split('(')[1].split(')')[0].split(',');fetch('/power?r='+a[0]+'&g='+a[1]+'&b='+a[2])}s.addColorStop(0,'rgba(255, 0, 0, 1)'),s.addColorStop(.17,'rgba(255, 255, 0, 1)'),s.addColorStop(.34,'rgba(0, 255, 0, 1)'),s.addColorStop(.51,'rgba(0, 255, 255, 1)'),s.addColorStop(.68,'rgba(0, 0, 255, 1)'),s.addColorStop(.85,'rgba(255, 0, 255, 1)'),s.addColorStop(1,'rgba(255, 0, 0, 1)'),r.fillStyle=s,r.fill(),d.addEventListener('click',function(t){c=t.offsetX,g=t.offsetY;var e=r.getImageData(c,g,1,1).data;b='rgba('+e[0]+','+e[1]+','+e[2]+',1)',u()},!1),t.addEventListener('mousedown',function(t){f=!0,p(t)},!1),t.addEventListener('mouseup',function(t){f=!1},!1),t.addEventListener('mousemove',function(t){f&&p(t)},!1)}),window.addEventListener('load',function(){document.getElementById('button-animation').addEventListener('click',function(){fetch('/animation')})});</script></head><body><header>LED-Streifen Kontrollzentrum</header><main><section><canvas id='color-block' height='300' width='300'></canvas><canvas id='color-strip' height='300' width='60'></canvas><br><p>Ausgew&auml;hlte Farbe:</p><canvas id='color-label' height='20' width='20'></canvas></section><input type='button' id='button-animation' value='Starte Animation'></main></body></html>");
}


// when new color received, activate it
void handlePower( void ) {
  animation_running = false;

  analogWrite(PinRed, server.arg("r").toInt() * 4);
  analogWrite(PinGreen, server.arg("g").toInt() * 4);
  analogWrite(PinBlue, server.arg("b").toInt() * 4);

  // ACK
  server.send ( 201, "text/plain", "");
}


// when the client tells the server to start an animation, do this
void handleAnimation( void ) {
  // start the animation
  animation_running = true;
  lastAnimPoll = millis();

  // init animation colors
  animColors[0] = 255;
  animColors[1] = 0;
  animColors[2] = 0;

  animState = 0;

  // ACK
  server.send ( 201, "text/plain", "");
}



// ---------------------------- The animation ----------------------------
void animation() {
  switch (animState) {

    // green is ingreasing until yellow
    case 0:
      animColors[1]++;
      if (animColors[1] >= 255)
        animState++;
      break;

    // red is degreesing until green
    case 1:
      animColors[0]--;
      if (animColors[0] <= 0)
        animState++;
      break;

    // blue is ingreasing until turquise
    case 2:
      animColors[2]++;
      if (animColors[2] >= 255)
        animState++;
      break;

    // green is degreesing until blue
    case 3:
      animColors[1]--;
      if (animColors[1] <= 0)
        animState++;
      break;

    // red is ingreasing until violet
    case 4:
      animColors[0]++;
      if (animColors[0] >= 255)
        animState++;
      break;

    // blue is degreesing until red
    case 5:
      animColors[2]--;
      if (animColors[2] <= 0)
        animState = 0;;
      break;
  }
}

