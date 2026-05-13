#include <Arduino.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define DEBUG_MODE

#define TDS_SENSOR A0
#define SW1 D3
#define TEMP_SENSOR D4

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

OneWire oneWire(TEMP_SENSOR);
DallasTemperature tempSensor(&oneWire);

ESP8266WebServer server(80);

const char* ssid = "MonsterchipRP";
const char* password = "170845G718";

static const unsigned char monsterchip_logo_bits[] = {0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x7c,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xff,0x00,0x00,0x80,0xff,0x00,0x00,0x80,0xff,0x00,0x00,0xc0,0xff,0x01,0x00,0xa0,0xff,0x01,0x00,0xb0,0xff,0x01,0x00,0xb8,0xf7,0x03,0x00,0x7c,0xf3,0x03,0x00,0x7c,0xf1,0x0f,0x00,0x7e,0xe0,0xff,0x00,0x7e,0xe0,0xff,0x1f,0x7e,0xe0,0xff,0xff,0x3e,0xc0,0xff,0x7f,0x3f,0x00,0xff,0x3f,0x3f,0x00,0xe0,0x3f,0x3f,0x00,0xe0,0x1f,0x3e,0x00,0xf0,0x0f,0x7e,0x00,0xf8,0x07,0xfe,0x00,0xfc,0x03,0xfc,0x01,0xf0,0x01,0xfc,0x83,0x0f,0x00,0xf8,0xff,0x7f,0x00,0xf8,0xff,0x3f,0x00,0xf0,0xff,0x1f,0x00,0xc0,0xff,0x07,0x00,0x00,0xff,0x01,0x00};
static const unsigned char image_Charging_lightning_mask_bits[] = {0x80,0x00,0x40,0x00,0x20,0x00,0x10,0x00,0x78,0x00,0x3c,0x00,0x10,0x00,0x08,0x00,0x04,0x00,0x02,0x00};
static const unsigned char image_weather_humidity_bits[] = {0x20,0x00,0x20,0x00,0x30,0x00,0x70,0x00,0x78,0x00,0xf8,0x00,0xfc,0x01,0xfc,0x01,0x7e,0x03,0xfe,0x02,0xff,0x06,0xff,0x07,0xfe,0x03,0xfe,0x03,0xfc,0x01,0xf0,0x00};
static const unsigned char image_weather_humidity_white_bits[] = {0x20,0x00,0x20,0x00,0x30,0x00,0x50,0x00,0x48,0x00,0x88,0x00,0x04,0x01,0x04,0x01,0x82,0x02,0x02,0x03,0x01,0x05,0x01,0x04,0x02,0x02,0x02,0x02,0x0c,0x01,0xf0,0x00};
static const unsigned char image_weather_temperature_bits[] = {0x38,0x00,0x44,0x40,0xd4,0xa0,0x54,0x40,0xd4,0x1c,0x54,0x06,0xd4,0x02,0x54,0x02,0x54,0x06,0x92,0x1c,0x39,0x01,0x75,0x01,0x7d,0x01,0x39,0x01,0x82,0x00,0x7c,0x00};

float water_temp = 0.0;
float electrical_conductivity = 0.0;
float ppm = 0.0;

void start_sequence(void);
void sample(uint8_t);
void draw_display(void); 

void handleRoot();
void handleData();

void setup() {
    start_sequence();
}

void loop() {
    server.handleClient();

    sample(2);
    draw_display();
}

void wifi_setup(){
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.drawStr(6, 29, "CONNECTING TO");
    u8g2.drawStr(33, 45, "WiFi...");
    u8g2.drawLine(0, 31, 127, 31);

    u8g2.sendBuffer();

    #ifdef DEBUG_MODE
        Serial.print("Connecting");
    #endif

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);

        #ifdef DEBUG_MODE
            Serial.print(".");
        #endif
    }

    #ifdef DEBUG_MODE
        Serial.println("");
        Serial.println("Connected!");
        Serial.println(WiFi.localIP());
    #endif

    server.on("/", handleRoot);
    server.on("/data", handleData);

    server.begin();

    String ip = WiFi.localIP().toString();

    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.drawStr(24, 20, "CONNECTED");
    u8g2.drawLine(0, 24, 127, 24);
    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.drawStr(3, 40, "IP ADDRESS:");
    u8g2.drawStr(3, 55, ip.c_str());

    u8g2.sendBuffer();

    delay(2000);
}

void start_sequence(void){
    #ifdef DEBUG_MODE
        Serial.begin(115200);
    #endif
    
    pinMode(SW1, INPUT_PULLUP);
    pinMode(TDS_SENSOR, INPUT);

    tempSensor.begin();
    tempSensor.setWaitForConversion(false);
    tempSensor.setResolution(12);

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setDrawColor(2);
    u8g2.drawBox(0, 0, 128, 63);

    u8g2.drawXBM(14, 16, 32, 32, monsterchip_logo_bits);

    u8g2.setFont(u8g2_font_profont29_tr);
    u8g2.drawStr(60, 37, "IoT");

    u8g2.setFont(u8g2_font_profont10_tr);
    u8g2.drawStr(51, 47, "WATER MONITOR");

    u8g2.sendBuffer();

    delay(2500);

    wifi_setup();
}

void sample(uint8_t sample_s){
    static uint32_t last_sample_tick = 0;

    static uint32_t temp_request_tick = 0;
    static bool temp_requested = false;

    uint32_t current_tick = millis();

    if(!temp_requested){
        tempSensor.requestTemperatures();
        temp_request_tick = current_tick;
        temp_requested = true;
    }

    if(temp_requested && (current_tick - temp_request_tick >= 100)){
        float temp = tempSensor.getTempCByIndex(0);

        if(temp != DEVICE_DISCONNECTED_C){
            water_temp = temp;
        }

        temp_requested = false;
    }

    if(current_tick - last_sample_tick >= (sample_s * 1000)){
        last_sample_tick = current_tick;

        int raw = analogRead(TDS_SENSOR);
        float voltage = raw * (3.3 / 1023.0);
        electrical_conductivity = voltage * 2.0;
        ppm = electrical_conductivity * 500.0;

        #ifdef DEBUG_MODE
            Serial.print("TEMP: ");
            Serial.print(water_temp);
            Serial.println(" C");

            Serial.print("EC: ");
            Serial.print(electrical_conductivity);
            Serial.println(" mS/cm");

            Serial.print("PPM: ");
            Serial.print(ppm);
            Serial.println(" ppm");

            Serial.println();
        #endif
    }
}

void draw_display(void){
    u8g2.clearBuffer();

    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    if(digitalRead(SW1) == 0){
        String ip = WiFi.localIP().toString();

        u8g2.setFont(u8g2_font_profont17_tr);
        u8g2.drawStr(3, 16, "DEVICE IP:");
        u8g2.drawLine(0, 20, 127, 20);
        u8g2.setFont(u8g2_font_profont17_tr);
        u8g2.drawStr(3, 38, ip.c_str());
    }else{
        u8g2.drawXBM(5, 3, 16, 16, image_weather_temperature_bits);
        u8g2.drawXBM(5, 44, 11, 16, image_weather_humidity_bits);
        u8g2.drawXBM(5, 24, 11, 16, image_weather_humidity_white_bits);
        u8g2.drawXBM(5, 29, 9, 10, image_Charging_lightning_mask_bits);

        u8g2.setFont(u8g2_font_4x6_tr);
        u8g2.drawStr(21, 19, "TEMP:");
        u8g2.drawStr(21, 40, "EC:");
        u8g2.drawStr(21, 60, "TDS:");

        //data_placeholder
        u8g2.setFont(u8g2_font_profont17_tr);

        char temp_buffer[16];
        char ec_buffer[16];
        char ppm_buffer[16];

        snprintf(temp_buffer, sizeof(temp_buffer), "%.1fC", water_temp);
        snprintf(ec_buffer, sizeof(ec_buffer), "%.2fmS/cm", electrical_conductivity);
        snprintf(ppm_buffer, sizeof(ppm_buffer), "%.0fppm", ppm);

        u8g2.drawStr(41, 19, temp_buffer);
        u8g2.drawStr(33, 40, ec_buffer);
        u8g2.drawStr(37, 60, ppm_buffer);
    }

    u8g2.sendBuffer();
}

void handleRoot(){

    String html = R"rawliteral(

<!DOCTYPE html>
<html>
<head>

<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>Water Quality Monitor</title>

<style>

body{
    background:#0f172a;
    color:white;
    font-family:Arial;
    text-align:center;
    margin:0;
    padding:20px;
}

h1{
    margin-bottom:30px;
}

.container{
    display:flex;
    flex-wrap:wrap;
    justify-content:center;
    gap:20px;
}

.card{
    background:#1e293b;
    width:250px;
    padding:20px;
    border-radius:20px;
    box-shadow:0 0 20px rgba(0,0,0,0.3);
}

.label{
    font-size:18px;
    opacity:0.8;
}

.value{
    font-size:42px;
    margin-top:10px;
    font-weight:bold;
}

.unit{
    font-size:18px;
}

.footer{
    margin-top:40px;
    opacity:0.6;
}

</style>
</head>

<body>

<h1>🌊 IoT Water Quality Monitor</h1>

<div class="container">

    <div class="card">
        <div class="label">Temperature</div>
        <div class="value">
            <span id="temp">0</span>
            <span class="unit">°C</span>
        </div>
    </div>

    <div class="card">
        <div class="label">EC</div>
        <div class="value">
            <span id="ec">0</span>
            <span class="unit">mS/cm</span>
        </div>
    </div>

    <div class="card">
        <div class="label">TDS</div>
        <div class="value">
            <span id="ppm">0</span>
            <span class="unit">ppm</span>
        </div>
    </div>

</div>

<div class="footer">
    Uptime: <span id="uptime">0</span> sec
</div>

<script>

async function updateData(){

    try{

        const response = await fetch('/data');
        const data = await response.json();

        document.getElementById('temp').innerText = data.temperature;
        document.getElementById('ec').innerText = data.ec;
        document.getElementById('ppm').innerText = data.ppm;
        document.getElementById('uptime').innerText = data.uptime;

    }catch(e){
        console.log(e);
    }
}

setInterval(updateData, 2000);

updateData();

</script>

</body>
</html>

)rawliteral";

    server.send(200, "text/html", html);
}

void handleData(){

    String json = "{";

    json += "\"temperature\":";
    json += String(water_temp, 1);
    json += ",";

    json += "\"ec\":";
    json += String(electrical_conductivity, 2);
    json += ",";

    json += "\"ppm\":";
    json += String(ppm, 0);
    json += ",";

    json += "\"uptime\":";
    json += String(millis() / 1000);

    json += "}";

    server.send(200, "application/json", json);
}