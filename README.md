# Automatic Watering based on BigClown

This project solves a common problem of plant watering at home.
It is based on a feedback loop where the watering is initiated by the measurement of soil moisture.
The moisture is measured each 10 seconds. Besides that the temperature neat the plant is also measured.
The watering itself is done by a small 12v pump.
The water level in the water reservoir is measured so that when there is no water the watering is stopped.
All the actions that are done are send using BigClown module to the hub where the results can be analyzed.


![alt text](img/whole_small.jpg)

## Required components
* BigClown core module ![alt text]
![alt whole system](img/system_small.jpg)

* Soil moisture sensor
![alt moisture sensor](img/moisture_small.jpg)

* 12V pump
![alt pump](img/pump_small.jpg)

* Power source - a battery pack in my case
* Water level sensor
![alt water level sensor](img/water_level_small.jpg)

* Few electronic parts
* Water reservoir

## Schema
![alt schema](img/board.png)

```
