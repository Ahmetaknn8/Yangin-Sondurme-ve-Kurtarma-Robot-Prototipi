#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>

#define LIGHT_PIN 4

const int PWMFreq = 1000;
const int PWMResolution = 8;
const int PWMSpeedChannel = 2;
const int PWMLightChannel = 3;

#define PWDN_GPIO_NUM      32
#define RESET_GPIO_NUM     -1
#define XCLK_GPIO_NUM       0
#define SIOD_GPIO_NUM      26
#define SIOC_GPIO_NUM      27
#define Y9_GPIO_NUM        35
#define Y8_GPIO_NUM        34
#define Y7_GPIO_NUM        39
#define Y6_GPIO_NUM        36
#define Y5_GPIO_NUM        21
#define Y4_GPIO_NUM        19
#define Y3_GPIO_NUM        18
#define Y2_GPIO_NUM         5
#define VSYNC_GPIO_NUM     25
#define HREF_GPIO_NUM      23
#define PCLK_GPIO_NUM      22

const char* ssid     = "Ahmet_Akin";
const char* password = "20200855604";

AsyncWebServer server(80);
AsyncWebSocket wsCamera("/Camera");
AsyncWebSocket wsCarInput("/CarInput");
uint32_t cameraClientId = 0;
const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE( 
<!DOCTYPE html>
<html>
  <head>
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no"
    />
    <title>Esp32 Cam Kontrol</title>
    <style>
      * {
        box-sizing: border-box;
        font-size: 62.5%;
      }

      body {
        margin: 0;
        padding: 0;
        overflow-x: hidden;

        font-family: sans-serif;
      }

      .noselect { /* HTML  elementinin kullanıcı tarafından seçilememsini sağlar */
        -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
        -khtml-user-select: none; /* Konqueror HTML */
        -moz-user-select: none; /* Firefox */
        -ms-user-select: none; /* Internet Explorer/Edge */
        user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
      }

      .slidecontainer {
        width: 100%;
      }

      .controls {
        position: relative;
        display: flex;
        flex-direction: column;
        align-items: center;

        width: 100%;
        margin-bottom: 20px;
        padding-top: 3.2rem;
      }

      div p {
        padding-top: 0.4rem;
        text-align: center;
      }

      .controller { 
        display: none; 
      }
      
      .direction, .arrow { 
        display: none; 
      }

      #container {
        width: 100vw;
        height: 100vh;

        display: flex;
        flex-direction: column;
        gap: 0.8rem;

        padding: 1.6rem;

        background-color: #2f3640;
      }

      #cameraImage {
        width: 100%;
        height: max-content;
        object-fit: cover;
        aspect-ratio: 16 / 9;

        transform: scaleY(-1);

        background-color: #00ff00;
        border: 5px solid #00ff00;
        border-radius: 0.8rem;
      }

      h2 {
        font-size: 2.4rem;
        color: #f5f6fa;
        margin: 0;
        padding: 0;
      }

      p {
        font-size: 1.6rem;
        color: #dcdde1;
        margin: 0;
      }

      .hidden {
        display: none;
      }

      .slidecontainer {
        display: flex;
        flex-direction: column;
        gap: 0.8rem;
        margin-top: 1.6rem;
      }

      label {
        font-size: 1.6rem;
        color: #f5f6fa;
        margin-bottom: 0.4rem;
      }

      .slider {
        -webkit-appearance: none;
        width: 100%;
        height: 15px;
        border-radius: 8rem;
        background: #7f8fa6;
        outline: none;
        opacity: 0.7;
        -webkit-transition: 0.2s;
        transition: opacity 0.2s;
      }

      .slider:hover {
        opacity: 1;
      }
    </style>
  </head>
  <body class="noselect">
    <main id="container" style="overflow-y: scroll">
      <img id="cameraImage" src="" />

      <div id="controls" class="controls">
        <div class="controller">
        </div>

        <div class="slidecontainer">
          <label for="Light">Isik</label>
          <input
            type="range"
            min="0"
            max="255"
            value="0"
            class="slider"
            id="Light"
            oninput='sendButtonInput("Light",value)'
          />
        </div>

        <div
          style="
            width: 100%;
            display: flex;
            flex-direction: row;
            justify-content: space-between;
            align-items: center;
            padding-top: 1.6rem;
          "
        >
          <h2>Ahmet AKIN</h2>
          <p>20200855604</p>
        </div>
      </div>
    </main>

    <script>
      var webSocketCameraUrl = "ws:\/\/" + window.location.hostname + "/Camera";
      var webSocketCarInputUrl =
        "ws:\/\/" + window.location.hostname + "/CarInput";
      var websocketCamera;
      var websocketCarInput;

      const controls = document.getElementById("controls");
      const settings = document.getElementById("settings");

      function initCameraWebSocket() {
        websocketCamera = new WebSocket(webSocketCameraUrl);
        websocketCamera.binaryType = "blob";
        websocketCamera.onopen = function (event) {};
        websocketCamera.onclose = function (event) {
          setTimeout(initCameraWebSocket, 2000);
        };
        websocketCamera.onmessage = function (event) {
          var imageId = document.getElementById("cameraImage");
          imageId.src = URL.createObjectURL(event.data);
        };
      }

      function initCarInputWebSocket() {
        websocketCarInput = new WebSocket(webSocketCarInputUrl);
        websocketCarInput.onopen = function (event) {
          sendButtonInput("Light", document.getElementById("Light").value);
        };
        websocketCarInput.onclose = function (event) {
          setTimeout(initCarInputWebSocket, 2000);
        };
        websocketCarInput.onmessage = function (event) {};
      }

      function initWebSocket() {
        initCameraWebSocket();
        initCarInputWebSocket();
      }

      function sendButtonInput(key, value) {
        var data = key + "," + value;
        websocketCarInput.send(data);
      }

      window.onload = initWebSocket;
      document
        .getElementById("container")
        .addEventListener("touchend", function (event) {
          event.preventDefault();
        });
    </script>
  </body>
</html>
)HTMLHOMEPAGE";


void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server, 
                              AsyncWebSocketClient *client,   
                              AwsEventType type, 
                              void *arg, 
                              uint8_t *data, 
                              size_t len) 
{ 
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      ledcWrite(PWMLightChannel, 0); 
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info; 
      info = (AwsFrameInfo*)arg; 

      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = ""; 
        myData.assign((char *)data, len); 

        std::istringstream ss(myData); 
        std::string key, value; 
        std::getline(ss, key, ','); 
        std::getline(ss, value, ','); 
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str()); 
        int valueInt = atoi(value.c_str());       
        
        if (key == "Light")
        {
          ledcWrite(PWMLightChannel, valueInt);         
        }             
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void onCameraWebSocketEvent(AsyncWebSocket *server, 
                            AsyncWebSocketClient *client, 
                            AwsEventType type,
                            void *arg, 
                            uint8_t *data, 
                            size_t len) 
{ 
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      cameraClientId = 0; 
      break;
    case WS_EVT_DATA:
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void setupCamera()
{
  camera_config_t config; 
  config.ledc_channel = LEDC_CHANNEL_4;
  config.ledc_timer = LEDC_TIMER_2;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  config.frame_size = FRAMESIZE_VGA; 
  config.jpeg_quality = 10; 
  config.fb_count = 1; 


  esp_err_t err = esp_camera_init(&config); 

  if (err != ESP_OK) 
  {
    Serial.printf("Kamera baslatma hatasi: 0x%x", err); 
    return; 
  }   

  if (psramFound()) 
  {
    heap_caps_malloc_extmem_enable(20000); 

    Serial.printf("PSRAM baslatildi. Bu boyutun uzerindeki bellek PSRAM'den alinacak");    
  }   
}

void sendCameraPicture() 
{
  if (cameraClientId == 0) 
  {
    return;
  }
  unsigned long  startTime1 = millis(); 
  camera_fb_t * fb = esp_camera_fb_get(); 
  if (!fb) 
  {
      Serial.println("Frame arabellegi alinamadi");
      return;
  }

  unsigned long  startTime2 = millis();
  wsCamera.binary(cameraClientId, fb->buf, fb->len); 

  esp_camera_fb_return(fb); 
    
  while (true)
  {
    AsyncWebSocketClient * clientPointer = wsCamera.client(cameraClientId); 

    if (!clientPointer || !(clientPointer->queueIsFull())) 
    {
      break;
    }
    delay(1); 
  }
  
  unsigned long  startTime3 = millis();  
  Serial.printf("Gecen Sure Toplam: %d|%d|%d\n",startTime3 - startTime1, startTime2 - startTime1, startTime3-startTime2 );
}

void setUpPinModes()
{
  ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution); 
  ledcSetup(PWMLightChannel, PWMFreq, PWMResolution);
        
  pinMode(LIGHT_PIN, OUTPUT);     
  ledcAttachPin(LIGHT_PIN, PWMLightChannel); 
}


void setup(void) 
{
  setUpPinModes();

  WiFi.softAP(ssid, password); 
  IPAddress IP = WiFi.softAPIP(); 
  Serial.print("Erisim Noktasi IP adresi:  "); 
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);   
  server.onNotFound(handleNotFound);

  wsCamera.onEvent(onCameraWebSocketEvent);   
  server.addHandler(&wsCamera);

  wsCarInput.onEvent(onCarInputWebSocketEvent);   
  server.addHandler(&wsCarInput);
  server.begin();
  Serial.println("HTTP sunucusu baslatildi"); 

  setupCamera();
}


void loop() 
{
  wsCamera.cleanupClients(); 
  wsCarInput.cleanupClients();
  sendCameraPicture(); 
  Serial.printf("SSPIRam Toplam bellek  %d, SPIRam Bos Bellek  %d\n", ESP.getPsramSize(), ESP.getFreePsram());
}