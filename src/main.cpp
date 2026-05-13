#include <Arduino.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG_MODE

#define TDS_SENSOR A0
#define SW1 D3
#define TEMP_SENSOR D4

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

OneWire oneWire(TEMP_SENSOR);
DallasTemperature tempSensor(&oneWire);

static const unsigned char monsterchip_logo_bits[] = {0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x7c,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0xff,0x00,0x00,0x80,0xff,0x00,0x00,0x80,0xff,0x00,0x00,0xc0,0xff,0x01,0x00,0xa0,0xff,0x01,0x00,0xb0,0xff,0x01,0x00,0xb8,0xf7,0x03,0x00,0x7c,0xf3,0x03,0x00,0x7c,0xf1,0x0f,0x00,0x7e,0xe0,0xff,0x00,0x7e,0xe0,0xff,0x1f,0x7e,0xe0,0xff,0xff,0x3e,0xc0,0xff,0x7f,0x3f,0x00,0xff,0x3f,0x3f,0x00,0xe0,0x3f,0x3f,0x00,0xe0,0x1f,0x3e,0x00,0xf0,0x0f,0x7e,0x00,0xf8,0x07,0xfe,0x00,0xfc,0x03,0xfc,0x01,0xf0,0x01,0xfc,0x83,0x0f,0x00,0xf8,0xff,0x7f,0x00,0xf8,0xff,0x3f,0x00,0xf0,0xff,0x1f,0x00,0xc0,0xff,0x07,0x00,0x00,0xff,0x01,0x00};
static const unsigned char image_Charging_lightning_mask_bits[] = {0x80,0x00,0x40,0x00,0x20,0x00,0x10,0x00,0x78,0x00,0x3c,0x00,0x10,0x00,0x08,0x00,0x04,0x00,0x02,0x00};
static const unsigned char image_weather_humidity_bits[] = {0x20,0x00,0x20,0x00,0x30,0x00,0x70,0x00,0x78,0x00,0xf8,0x00,0xfc,0x01,0xfc,0x01,0x7e,0x03,0xfe,0x02,0xff,0x06,0xff,0x07,0xfe,0x03,0xfe,0x03,0xfc,0x01,0xf0,0x00};
static const unsigned char image_weather_humidity_white_bits[] = {0x20,0x00,0x20,0x00,0x30,0x00,0x50,0x00,0x48,0x00,0x88,0x00,0x04,0x01,0x04,0x01,0x82,0x02,0x02,0x03,0x01,0x05,0x01,0x04,0x02,0x02,0x02,0x02,0x0c,0x01,0xf0,0x00};
static const unsigned char image_weather_temperature_bits[] = {0x38,0x00,0x44,0x40,0xd4,0xa0,0x54,0x40,0xd4,0x1c,0x54,0x06,0xd4,0x02,0x54,0x02,0x54,0x06,0x92,0x1c,0x39,0x01,0x75,0x01,0x7d,0x01,0x39,0x01,0x82,0x00,0x7c,0x00};

float water_temp, electrical_conductivity, ppm;

void start_sequence(void);
void sample(uint8_t);
void draw_display(void); 

void setup() {
    start_sequence();
}

void loop() {
    sample(2);
    draw_display();
}

void wifi_setup(void){
    //wifi_stuff tbd
}

void start_sequence(void){
    #ifdef DEBUG_MODE
        Serial.begin(115200);
    #endif
    
    pinMode(SW1, INPUT_PULLUP);
    pinMode(TDS_SENSOR, INPUT);

    tempSensor.begin();

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

    //wifi_setup();
}

void sample(uint8_t sample_s){
    uint32_t current_tick = millis();
    static uint32_t last_tick = 0;

    if(current_tick - last_tick > (sample_s * 1000)){
        last_tick = current_tick;

        tempSensor.requestTemperatures();
        water_temp = tempSensor.getTempCByIndex(0);

        int raw = analogRead(TDS_SENSOR);

        float voltage = raw * (3.3 / 1023.0);

        electrical_conductivity = voltage * 2.0;

        // convert mS/cm -> ppm
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
        u8g2.setFont(u8g2_font_profont17_tr);

        // String ip = WiFi.localIP().toString();

        u8g2.drawStr(3, 16, "DEVICE IP:");
        // u8g2.drawStr(3, 36, ip.c_str());
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