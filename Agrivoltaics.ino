#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <Dramco-UNO.h>

#define HUMIDITYLOWSAFE (68)
#define HUMIDITYHIGHSAFE (82)
#define HUMIDITYLOWCRIT (65)
#define HUMIDITYHIGHCRIT (85)
#define LUMINOSITYLOW (300)
#define LUMINOSITYHIGH (800)
#define SAFELUMTEMPLOW (24)
#define SAFELUMTEMPHIGH (30)
#define NIGHTTHRESHOLD (150)
#define SAFELUMTEMPLOWNIGHT (13)
#define SAFELUMTEMPHIGHNIGHT (20)
#define LUMINOSNIGHTTHR (150)

BH1750 lightMeter(0x23);

Adafruit_BME280 bme;
int temperature = 0;
int humidity = 0;
const int AirValue = 360;   //Moisture in air
const int WaterValue = 150;  //Moisture in the water
int intervals = (AirValue - WaterValue)/3;
int soilMoistureValue = 0;
int solarPanelState = 0;
int waterFlowReq = 0;
int ventReq = 0;
int sunnyDay = 0;
int cloudyWeather = 0;
int dayTime = 0;
int lightIntensity = 0;
int heaterState = 0;
int artLightState = 0;
int timePanelOpen = 0;
bool humidityControl = 0;
int countMidnight = 0;
bool nightMode = 0;
float lux = 0;
int firstVal = 0;

void setup() {
Serial.begin(9600);
Wire.begin();
if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
 /* Serial.println(F("BH1750 Advanced begin"));*/
}
else {
  Serial.println(F("Error initialising BH1750"));
}

if (!bme.begin(0x76)) {
  Serial.println("Could not find a valid BME280 sensor, check wiring!");
  while (1);
}
}

void loop() {
if(lux < NIGHTTHRESHOLD)
{
  nightMode = 1;
  
}
else
{
  nightMode = 0;
}
if(firstVal == 0)
{
  lightMeasureControl();
  firstVal++;
}

if(nightMode == 0)
{
  humidityControlMeasurement();
  lightMeasureControl(); 
 delay(100);
 DramcoUno.sleep(8000);
   
}
else
{
  humidityControlMeasurement();
  countMidnight++;
  if(countMidnight == 4)
  {
    countMidnight = 0; 
    lightMeasureControl(); 
  }
 delay(100);
 DramcoUno.sleep(8000);
}
}


/*Measure temperature*/
int measure_temperature()
{
  return bme.readTemperature(); //Measuring the temperature
}

/*Measure humidity*/
int measure_humidity()
{
  return bme.readHumidity(); //Measuring the temperature
}

/*Measure soil moisture*/
int measure_soil_moisture()
{
  return analogRead(1); //Measuring the temperature
}

/*Measure light intensity*/
float measure_light_intensity()
{
  return lightMeter.readLightLevel(); //Measuring the temperature
}

/*Temperature management*/
void temperature_management(int lowThresh, int highThresh, int caseVal)
{
  temperature = measure_temperature(); //Measuring the temperature
  Serial.print("Current temperature is ");
  Serial.print(temperature);
  Serial.println("degC");
  if(caseVal == 0)
  {
    if(lowThresh < temperature && temperature < highThresh)
    {
      if(solarPanelState == 1)
      {
        Serial.println("Close the solar panel optimal light being recieved");
        solarPanelState = 0;
      }
      if(artLightState == 1)
      {
        Serial.println("switching off artificial light");
        artLightState = 0;
      }
      Serial.println("Optimal temperature and light");
      if(heaterState)
      {
        Serial.println("Heater switched off");
      }
    }
    else
    {
      if(!heaterState)
      {
        Serial.println("switching on heater");
      }
      Serial.println("Heater mode ON");
    }
  }

  else if(caseVal == 1)
  {
    if(lowThresh < temperature && temperature < highThresh)
    {
      Serial.println("Closing the solar panel to reduce temperature");
      solarPanelState = 0;
      if(artLightState == 1)
       {
        Serial.println("switching off artificial light");
        artLightState = 0;
      }
      if(heaterState)
      {
        Serial.println("Heater switched off");
      }
     }
    else
    {
      if(!heaterState)
      {
        Serial.println("switching on heater");
      }
      Serial.println("Heater mode ON");
    }
  }

  else if(caseVal == 2)
  {
    if(lowThresh < temperature && temperature < highThresh)
    {
      if(artLightState == 0)
      {
        Serial.println("switching on artificial light");
        artLightState = 1;
      }
      if(heaterState)
      {
        Serial.println("Heater switched off");
      }
    }
    else
    {
      if(!heaterState)
      {
        Serial.println("switching on heater");
      }
      Serial.println("Heater mode ON");
    }
  }

  else if(caseVal == 3)
  {
    if(lowThresh < temperature && temperature < highThresh)
  {
      Serial.println("Plant is in sleep mode");
      if(heaterState)
      {
        Serial.println("Heater switched off");
      }
  }
    else
    {
      if(!heaterState)
      {
        Serial.println("switching on heater");
      }
      Serial.println("Heater mode ON");
    }
  }

}

/*Soil moisture Control*/
void soilMoistureMeasurement()
{
soilMoistureValue = measure_soil_moisture();  //put Sensor insert into soil
if((soilMoistureValue > WaterValue) && soilMoistureValue < (WaterValue + intervals))
{
  /*Serial.println("Very Wet");*/
  if(waterFlowReq == 1)
  {
    Serial.println("Water supply to plants will be switched off");
    waterFlowReq = 0;
  }
}
else if((soilMoistureValue > (WaterValue + intervals)) && soilMoistureValue < (AirValue - intervals))
{
  /*Serial.println("Wet");*/
  if(waterFlowReq == 1)
  {
    Serial.println("Consider switching off the water supply");
  }
}
else if (soilMoistureValue > (AirValue - intervals) && soilMoistureValue < AirValue)
{
  /*Serial.println("Dry");*/
  if(waterFlowReq == 0)
  {
    Serial.println("Consider switching ON the water supply");
  }
}
else
{
  /*Serial.println("Very Dry");*/
  if(waterFlowReq == 0)
  {
    Serial.println("Water supply to plants will be switched ON");
    waterFlowReq = 1;
  }
}
}

/*Humidity measurement*/
void humidityControlMeasurement()
{
humidity = measure_humidity(); //Measuring the humidity 
Serial.print("Current humidity is ");
Serial.print(humidity);
Serial.println("%");
if(HUMIDITYLOWSAFE < humidity &&  humidity < HUMIDITYHIGHSAFE)
{
    if(humidityControl == 1 && humidity > HUMIDITYHIGHCRIT && heaterState == 0)
    {
      humidityControl = 0;
      Serial.println("Switching off humidity control");    
    }
    else if(humidityControl == 1 && humidity < HUMIDITYLOWCRIT && heaterState == 1)
    {
      humidityControl = 0;
      Serial.println("Switching off humidity control");    
    }
  
}
else
{
  if(HUMIDITYLOWCRIT < humidity && humidity < HUMIDITYHIGHCRIT)
  {

  }
  else
  {
    if(humidity < HUMIDITYLOWSAFE)
    {
      Serial.println("Moisture too low, starting the humidity controller");
      humidityControl = 1;
      heaterState = 0;
    }
    else if(humidity > HUMIDITYHIGHSAFE)
    {
      Serial.println("Moisture too high, starting the humidity controller");
      humidityControl = 1;
      heaterState = 1;
    }
  }
}
}

/*Light intensity measurement*/
void lightMeasureControl()
{
lux = measure_light_intensity();
Serial.print("Current light receieved is ");
Serial.print(lux);
Serial.println("lx");
if(LUMINOSITYLOW < lux && lux < LUMINOSITYHIGH)
{
  temperature_management(SAFELUMTEMPLOW, SAFELUMTEMPHIGH,0);
  soilMoistureMeasurement();
  Serial.println("Light is optimal");
}
else if(lux > LUMINOSITYHIGH)
{
  temperature_management(SAFELUMTEMPLOW, SAFELUMTEMPHIGH,1);
  soilMoistureMeasurement();
  Serial.println("Too much of light");
}
else if(lux < LUMINOSNIGHTTHR)
{
  temperature_management(SAFELUMTEMPLOWNIGHT, SAFELUMTEMPHIGHNIGHT,3);
  soilMoistureMeasurement();
  Serial.println("Night mode");
}
else
{
  temperature_management(SAFELUMTEMPLOW, SAFELUMTEMPHIGH,2);
  soilMoistureMeasurement();
  Serial.println("Artifical light mode");
}
}
