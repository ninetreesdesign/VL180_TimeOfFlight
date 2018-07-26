/* This example shows how to use continuous mode to take
range measurements with the VL6180X.
The range readings are in units of mm. */
// Sensor with some averaging is +-0.5mm anywhere within its range.
// not affected by ambient light
// however, significantly affected by temperature. a few degrees drop changes distance by 1mm;
// data sheet indicates ~15mm over its temperature range

// sometimes i2c scan fails after upload; must disconnect/reconnect usb port

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Adafruit_SSD1306-64.h>
#include "Adafruit_VL6180X.h"

Adafruit_VL6180X vl = Adafruit_VL6180X();

#define OLED_RESET 4
#define I2C_ADDR 0x3c   // 128x32 = 3c, blu/yel = 3c, 128x64=0x3D, but =3c if SA0 = GND
// swapped display positions to use a yel/blu 128x64    its addr is 3C instead of 3D adafruit default

Adafruit_SSD1306 display(OLED_RESET);
//Adafruit_SSD1306 display = Adafruit_SSD1306();

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

char buffer[40];

void setup()
{
    Serial.begin(57600); // rate doesn't matter for teensy routed to USB console
    while (!Serial && (millis() < 4000)) {
    } // include timeout if print console isn't opened

    i2cScan();        // requires serial port to be initialized prior to use

    display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR);  // initialize with the I2C addr 0x3C/3D (for the 128x32/64)
    // init done
    display.clearDisplay();
    display.display();
    delay(1000);
    display.setTextColor(WHITE);
    // display.setTextSize(4);         // text display big!
    display.setTextSize(2);
    display.setCursor(20,0);
    sprintf(buffer, "%7.1f",0);
    display.print(buffer);
    display.setTextSize(1);
    display.print(" lux");
    display.display();
    display.setTextSize(2);
    display.setCursor(0,22);
    display.print("TOF meas");
    display.setCursor(0,46);
    display.print("distance");
    display.display();
    delay(3000);
    display.setTextSize(3);
    display.setCursor(0,33);
    display.print("    ");
    display.setTextSize(2);
    display.print(" cm");


    Serial.print("Detect Adafruit VL6180x?  ");
    int i = 0;
    if (! vl.begin()) {
        Serial.println("Failed to find sensor");
        while ( i < 1000) {
            i++;
        }
    }
    else
        Serial.println("Sensor present.");

    display.clearDisplay();
    display.display();
}


void loop()
{
    int32_t tic;
    int32_t toc;
    static float range2 = 0;
    static float range1 = 0;
    static float range0 = 0;
    static float range;

    static float lux1 = 0;
    static float lux0 = 0;
    static float lux;

    lux1 = lux0;
    lux0 = vl.readLux(VL6180X_ALS_GAIN_5);
    lux = 0.7*lux0 + 0.3*lux1;

    range2 = range1;
    range1 = range0;
    range0 = 0;
    int n = 32;
    for (int i = 0; i<n; i++) {
        range0 += (float)vl.readRange();
    }
    range0 /= n;
    range = 0.7*range0 + 0.2*range1 + 0.1*range2;
    //range = median3(range0,range1,range2);
    range /= 10;        // to cm
    sprintf(buffer, "%04.2f",range);
    uint8_t status = vl.readRangeStatus();
    if (status == VL6180X_ERROR_NONE) {
        Serial.print("\trange [cm]\t");
        Serial.println(buffer);
        // print units
        display.clearDisplay();
        display.setTextSize(3);
        display.setCursor(0,33);
        display.print(buffer);  // range value
        display.setTextSize(2);
        display.print(" cm");

        printLux(lux);

    } else {
        Serial.println(" \terror: out of range?");
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0,33);
        display.print("err:range?");
        display.display();

        printLux(lux);
    }
    delay(10);
}


void printLux(float lux) {
    // print light level
    display.setTextSize(2);
    display.setCursor(20,0);
    sprintf(buffer, "%7.1f",lux);   // x.x to xxxx.x
    display.print(buffer);
    display.setTextSize(1);
    display.print(" lux");
    display.display();

    Serial.print("Lux\t ");
    Serial.print(lux,1);
}


// calculate the median from 3 adjacent points
double median3( double a0, double a1, double a2 ) {
    /* SWAP(1, 2); SWAP(0, 2); SWAP(0, 1); */
    swap(&a1, &a2); // swaps values  if arg1 > arg2
    swap(&a0, &a2);
    swap(&a0, &a1);

    return a1; // median value
}


//! --- swaps values (float/double) of j and k if j > k
void swap(double *j,  double *k) {
    double x = *j;
    double y = *k;
    if (*j > *k) {
        *k = x;
        *j = y;
    }
}


void i2cScan() {
    Serial.println ("\ni2cScan...\n");
    static int count = 0;

    Wire.begin();
    for (int i = 8; i < 120; i++)
    {
        Wire.beginTransmission(i);          // probe this i2c address
        if (Wire.endTransmission() == 0)
        {
            Serial.print ("  device at: 0x" + String(i, HEX) + "  [" + String(i) + "] \n");
            count++;
            delay (1);
        }
    }
    Serial.print("\nTotal devices:  " + String(count) + "\n");
}




