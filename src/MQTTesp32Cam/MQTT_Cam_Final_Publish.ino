#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
//cam
#include "esp_camera.h"
#include "esp_timer.h"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  

// Precompiled config
// #define CAMERA_MODEL_HEZHOU_ESP32_S3
#define CAMERA_MODEL_AI_THINKER

// #define mqttSSL
#define mqttLocal
#if defined (mqttSSL)
  const char* ca_cert= \    // EMQX Cloud CA
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
  "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
  "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
  "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
  "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
  "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
  "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
  "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
  "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
  "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
  "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
  "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
  "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
  "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
  "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
  "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
  "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
  "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=" \
  "-----END CERTIFICATE-----\n";

  WiFiClientSecure espClient;
  PubSubClient client(espClient);
    // MQTT SSL Broker
  const char *mqtt_broker = "Your SSL broker";
  const char *topic = "camera"; 
  const char *mqtt_username = "Your username"; 
  const char *mqtt_password = "Your password";
  int mqtt_port = 8883;
  const char *client_id = "espCam";
#elif defined (mqttLocal)
  WiFiClient espClient;
  PubSubClient client(espClient);
  // MQTT Broker
  const char *mqtt_broker = "Your broker";
  const char *topic = "camera"; 
  int mqtt_port = 1883;
  const char *client_id = "espCam";
#endif

#define maxRetry 20 
bool camstart = false;

//cam gpio
#if defined(CAMERA_MODEL_HEZHOU_ESP32_S3)
  #define PWDN_GPIO_NUM -1
  #define RESET_GPIO_NUM -1
  #define XCLK_GPIO_NUM 39
  #define SIOD_GPIO_NUM 21
  #define SIOC_GPIO_NUM 46

  #define Y2_GPIO_NUM 34
  #define Y3_GPIO_NUM 47
  #define Y4_GPIO_NUM 48
  #define Y5_GPIO_NUM 33
  #define Y6_GPIO_NUM 35
  #define Y7_GPIO_NUM 37
  #define Y8_GPIO_NUM 38
  #define Y9_GPIO_NUM 40

  #define VSYNC_GPIO_NUM 42
  #define HREF_GPIO_NUM 41
  #define PCLK_GPIO_NUM 36
#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27

  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#endif

// WiFi
const char *ssid = "Your WiFi SSID"; 
const char *psw = "Your WiFi psw"; 

void stubbornConnect()
{
  uint8_t wifi_retry_count = 0;
  Serial.println("start connect");
  while (WiFi.status() != WL_CONNECTED && wifi_retry_count < maxRetry) {
    WiFi.begin(ssid, psw ); // put your ssid / pass if required, only needed once
    Serial.print(WiFi.macAddress());
    Serial.printf(" => WiFi connect - Attempt No. %d\n", wifi_retry_count+1);
    delay(3000);
    wifi_retry_count++;
  }
  if(wifi_retry_count >= maxRetry ) {
    Serial.println("no connection, forcing restart");
    ESP.restart();
  } 
  if (WiFi.waitForConnectResult() == WL_CONNECTED){
    Serial.println("Connected as");
    Serial.println(WiFi.localIP());
  }
}

void mqttConnect(){
  uint8_t maxcount = 15;
  #if defined(mqttSSL) 
    while(!client.connect(client_id,mqtt_username,mqtt_password)){
      Serial.print(".");
      delay(1000);
      maxcount --;
      if(maxcount == 0){
        Serial.println("connect failed!");
        ESP.restart();
      }
    }
    client.subscribe(topic);
    Serial.println("Connected!");
  #else if defined(mqttLocal) 
    while(!client.connect(client_id)){
      Serial.print(".");
      delay(1000);
      maxcount --;
      if(maxcount == 0){
        Serial.println("connect failed!");
        ESP.restart();
      }
    }
    client.subscribe(topic);
    Serial.println("Connected!");
  #endif
}

void connect() {
  Serial.print("checking wifi...");
  if(WiFi.status() != WL_CONNECTED){
    stubbornConnect();
  }
  mqttConnect();
  client.subscribe(topic);
}

void getimg(){
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }
  else
  {
    Serial.println("Camera Captured");
  }
  client.beginPublish(topic, fb -> len, 0);
  client.write(fb -> buf,fb -> len);
  client.endPublish();
  client.loop();
  esp_camera_fb_return(fb);
  fb = NULL;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = String((char*)payload, length);
  if (msg == "camstart"){
    camstart = true;
    Serial.println("Camera start to work!");
  }elif(msg == "camstop"){
    camstart = false;
    Serial.println("Camera stop working!");
  }
}

void setup() {
  Serial.begin(115200);
  stubbornConnect();
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  // config.grab_mode = CAMERA_GRAB_LATEST;
  #if defined (CAMERA_MODEL_HEZHOU_ESP32_S3)
    config.grab_mode = config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.jpeg_quality = 60;
    config.fb_count = 1;
  #elif defined(CAMERA_MODEL_AI_THINKER)
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 30;
    config.fb_count = 3;
  #endif
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }else{
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    mqttConnect();
  }
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }
  if(camstart == true){
    getimg();
  }

  // delay(1000);
}
