#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h> // Adafruit TSL2561 by Adafruit Version 1.1.0
#include <SimpleDHT.h> // SimpleDHT by Winlin Version 1.0.12

#define DHTPIN 8 // Pin DTH22 is connected to 

const unsigned int REFRESH = 100; // Period to read sensors
const float ALPHA = 0.95; // Value for filter.

// Time tracker
unsigned long start = millis();

SimpleDHT22 dht22(DHTPIN);
/* DTH22 works with 3 to 5 V
 * Reads values through an analogous pin
 * Readings can be done every 2 seconds
 * Measures humidity from 0 to 100%
 * Precision of 2 to 5% accuracy
 * Measures temperatures fro -40 to 80 Celsius
 * Precision of 0.5% accuracy 
 */

// Variables for DTH22
float t1 = 30;
float h1 = 64;

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
/* TSL2591 works with 3.3V= or 5V
 * Reads values through IC2
 * Maximum Lux of 88L
 * Dynamic Range of 600M:1
 * Measures infrared and visible light
 */

// Variables for Adafruit_TSL2591
unsigned int lux1 = 50;

void configureTSL2561(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */  
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");
}


void setup() {
  Serial.begin(9600);

  // Initialize TSL2591
  if(tsl.begin()){
    Serial.println("Found TSL2561 sensor");
    // Configure tsl
    configureTSL2561();
  } 
  else{
    Serial.println("TSL2591 sensor not found");
    while (1);
  } 
  Serial.println("Temp *C\tHumidity %\tLux");
}


void loop() {
  unsigned long current = millis();

  if(current - start > REFRESH){ // Read from sensors every REFRESH millis
    // Starts reading and processing from DTH22
    float t2 = 0;
    float h2 = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht22.read2(&t2, &h2, NULL)) != SimpleDHTErrSuccess) {
      t2 = t1;
      h2 = h1;
    }    
    // Low pass filter
    float tF = ALPHA * t1 + (1 - ALPHA) * t2;
    float hF = ALPHA * h1 + (1 - ALPHA) * h2;
  
    // Update past values
    t1 = tF;
    h1 = hF;
  
    // Ends reading and processing from DTH11

    // Starts reading and processing from TSL2591X Light Sensor
    sensors_event_t event;
    tsl.getEvent(&event);
    unsigned int lux2;
    
    if (event.light) {
      lux2 = event.light;
    }
    else {
      lux2 = lux1;
    }
    
    // Low pass filter
    unsigned int luxF = ALPHA * lux1 + (1 - ALPHA) * lux2;

    // Update past values
    lux1 = luxF;

    Serial.print(tF);
    Serial.print("\t");
    Serial.print(hF);
    Serial.print("\t");
    Serial.println(luxF);
    
    start = millis();
  }

  
}
