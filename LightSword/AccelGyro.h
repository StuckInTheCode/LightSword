#pragma once
#include "MPU6050.h"        // accel-gyroscope library

class AccelGyro
{
public:
	//values of coordinates get from the sensor
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	unsigned long ACC, GYR;
	int gyrX, gyrY, gyrZ, accX, accY, accZ;
	unsigned long sensor_timer;
	//instance of the accel-gyroscope
	MPU6050 sensor;

	AccelGyro();
	~AccelGyro();

	boolean testConnection();
	void checkSensorState();
};

