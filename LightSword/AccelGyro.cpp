#include "AccelGyro.h"

AccelGyro::AccelGyro()
{
	sensor.initialize();			//setup sensor
	sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
	sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
}


AccelGyro::~AccelGyro()
{
}

boolean AccelGyro::testConnection()
{
	return sensor.testConnection();
}

void AccelGyro::checkSensorState()
{
	if (swordState) {
		if (millis() - sensor_timer > 600) {

			sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);       // every 300ms save checked values

			gyrX = abs(gx / 100);
			gyrY = abs(gy / 100);
			gyrZ = abs(gz / 100);
			accX = abs(ax / 100);
			accY = abs(ay / 100);
			accZ = abs(az / 100);

			ACC = sq((long)accX) + sq((long)accY) + sq((long)accZ);
			ACC = sqrt(ACC);
			GYR = sq((long)gyrX) + sq((long)gyrY) + sq((long)gyrZ);
			GYR = sqrt((long)GYR);

			sensor_timer = micros();
		}
	}
}
