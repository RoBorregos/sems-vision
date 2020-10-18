#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h" // // Adafruit TSL2591 Library by Adafruit Version 1.2.1
#include "DHT.h" // DHT sensor library by Adafruit Version 1.4.0
#define DHTPIN 2 // Pin DTH22 is connected to 
#define DHTTYPE DHT22

const unsigned int REFRESH = 2500; // Period to read sensors
const float ALPHA = 0.8; // Value for filter.

// Time tracker
unsigned long start = millis();

// Variables for DTH22
float h1 = 64;
float t1 = 30;
float f1 = 86;
float h2, t2, f2, hif, hic;

DHT dht(DHTPIN, DHTTYPE);
/* DTH22 works with 3 to 5 V
 * Reads values through an analogous pin
 * Readings can be done every 2 seconds
 * Measures humidity from 0 to 100%
 * Precision of 2 to 5% accuracy
 * Measures temperatures fro -40 to 80 Celsius
 * Precision of 0.5% accuracy 
 */


Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
/* TSL2591 works with 3.3V= or 5V
 * Reads values through IC2
 * Maximum Lux of 88L
 * Dynamic Range of 600M:1
 * Measures infrared and visible light
 */

// Variables for Adafruit_TSL2591
unsigned int lux1 = 300;

void configureTSL2591(void) // Edited function from example
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
}



void setup() {
  Serial.begin(9600);

  // Initialize DTH22
  dht.begin();

  // Initialize TSL2591
  if(tsl.begin()){
    Serial.println("Found TSL2591 sensor");
    // Configure tsl
    configureTSL2591();
  } 
  else{
    Serial.println("TSL2591 sensor not found");
    while (1);
  }
  
}


void loop() {
  
  unsigned long current = millis();
  
  if(start - current > REFRESH){ // Read from sensors every REFRESH millis
    // Starts reading and processing from DTH22
    h2 = dht.readHumidity(); // Read humidity
    t2 = dht.readTemperature(); // Read temperature in Celsius
    f2 = dht.readTemperature(true); // Read temperature in Fahrenheit
    
    if (isnan(h2) || isnan(t2) || isnan(f2)){ // Set to previous values
      h2 = h1;
      t2 = t1;
      f2 = f1;
    }
    // Ends reading from DTH22
    
    // Low pass filter
    float hF = ALPHA * h1 + (1 - ALPHA) * h2;
    float tF = ALPHA * t1 + (1 - ALPHA) * t2;
    float fF = ALPHA * f1 + (1 - ALPHA) * f2;
  
    // Update past values
    h1 = hF;
    t1 = tF;
    f1 = fF;
    
    hif = dht.computeHeatIndex(fF, hF); // Heat index in Fahrenheit
    hic = dht.computeHeatIndex(tF, hF, false); // Heat index in Celsius
    // Ends processing from DTH11

    // Starts reading and processing from TSL2591X Light Sensor

    unsigned int lum = tsl.getFullLuminosity();
    unsigned int ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    unsigned int lux2 = tsl.calculateLux(full, ir); // Lux Value

    // Low pass filter
    unsigned int luxF = ALPHA * lux1 + (1 - ALPHA) * lux2;

    // Update past values
    lux1 = luxF;
  }
  
}
