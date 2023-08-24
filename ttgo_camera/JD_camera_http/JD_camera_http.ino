#include <WiFi.h>
#include <Wire.h>
#include "esp_camera.h"
/***************************************
 *  Function
 **************************************/
//When using timed sleep, set the sleep time here
#define uS_TO_S_FACTOR 1000000              /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5                    /* Time ESP32 will go to sleep (in seconds) */

/***************************************
 *  WiFi
 **************************************/
#define WIFI_SSID   "GBSA0001"
#define WIFI_PASSWD "GBSA0001"

#include "select_pins.h"

String macAddress = "";
String ipAddress = "";

extern void startCameraServer();

#if defined(SSD130_MODLE_TYPE)
// Depend OLED library ,See  https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#define SSD1306_ADDRESS 0x3c
SSD1306 oled(SSD1306_ADDRESS, I2C_SDA, I2C_SCL, (OLEDDISPLAY_GEOMETRY)SSD130_MODLE_TYPE);
OLEDDisplayUi ui(&oled);
#endif


bool deviceProbe(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

bool setupDisplay()
{
    static FrameCallback frames[] = {
        [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
            display->setTextAlignment(TEXT_ALIGN_CENTER);
            display->setFont(ArialMT_Plain_10);

            display->drawString(64 + x, 9 + y, macAddress);
            display->drawString(64 + x, 25 + y, ipAddress);
        },
        [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
            display->setTextAlignment(TEXT_ALIGN_CENTER);
            display->setFont(ArialMT_Plain_10);
            display->drawString( 64 + x, 5 + y, "Camera Ready! Use");
            display->drawString(64 + x, 25 + y, "http://" + ipAddress );
            display->drawString(64 + x, 45 + y, "to connect");
        }
    };

    if (!deviceProbe(SSD1306_ADDRESS))return false;
    oled.init();
    // Wire.setClock(100000);  //! Reduce the speed and prevent the speed from being too high, causing the screen
    oled.setFont(ArialMT_Plain_16);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    // delay(50);
    oled.drawString( oled.getWidth() / 2, oled.getHeight() / 2 - 10, "LilyGo CAM");
    oled.display();
    ui.setTargetFPS(30);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, sizeof(frames) / sizeof(frames[0]));
    ui.setTimePerFrame(6000);
    ui.disableIndicator();
    return true;
}


void loopDisplay()
{
    ui.update();
}


bool setupPower()
{

    if (!deviceProbe(IP5306_ADDR))return false;
    bool en = true;
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
    return true;
}

bool setupCamera()
{
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
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);//flip it back
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }
    //drop down frame size for higher initial frame rate
    //s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_framesize(s, FRAMESIZE_VGA);
    return true;
}
WiFiServer wifiServer(83);
void setupNetwork()
{
    
    macAddress = "LilyGo-CAM-";
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    ipAddress = WiFi.localIP().toString();
    macAddress += WiFi.macAddress().substring(0, 5);
    wifiServer.begin();
}

void setup()
{
    // Start serial 
    Serial.begin(115200);
    // Start I2C bus 
    Wire.begin(I2C_SDA, I2C_SCL);
    // Display setup 
    bool status = setupDisplay();
    Serial.print("setupDisplay status "); 
    Serial.println(status);
    // Board power setup  
    status = setupPower();
    Serial.print("setupPower status "); 
    Serial.println(status);
    // Caemra setup
    status = setupCamera();
    Serial.print("setupCamera status "); 
    Serial.println(status);
    if (!status) {
        delay(10000);
        esp_restart();
    }
    // WiFi setup
    setupNetwork();
    // Start camera web server 
    startCameraServer();
    // Display camera connection guide 
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(ipAddress);
    Serial.println("' to connect");

}

void loop()
{
    // OLED update 
    ui.update();
    // Check WiFi client connected 
    WiFiClient client = wifiServer.available();
    if (client) {
        while (client.connected()) {
            while (client.available()>0) {
                char c = client.read();
                Serial.write(c);
            }
            delay(1);
        }
        client.stop();
        Serial.println("Client disconnected");
    }
}
