#include "JY61P.h"
#include "JY61P_dfs.h"
#include <string.h>
#include <Wire.h>

const uint8_t JY61P_save_conf_cmd[5] = {0xFF,0xAA,0x00,0x00,0x00};
const uint8_t JY61P_imu_cali_cmd[5]  = {0xFF,0xAA,0x01,0x01,0x00};
const uint8_t JY61P_mag_cali_cmd[5]  = {0xFF,0xAA,0x01,0x02,0x01};
const uint8_t JY61P_quit_cali_cmd[5] = {0xFF,0xAA,0x01,0x00,0x00};

CJY61P::CJY61P()
{
	lastTime = millis();
}

void CJY61P::attach(Stream & Serial_temp)
{
	Serial_ = &Serial_temp;
}

void CJY61P::startIIC(uint8_t address)
{
	address_ = address;
	transferMode_ = 1;
	Wire.begin();
}

bool CJY61P::readSerialData(uint8_t data)
{
	rxBuffer[rxCnt] = data;
	rxCnt++;
	if (rxBuffer[0] != 0x55) { //0x55
		rxCnt = 0;
		return false;
	}
	if (rxCnt<11) {
		return false;
	}
	rxCnt = 0;  //归零计数
	uint8_t sum = 0;
	for (uint8_t cnt = 0; cnt<10; cnt++) {
		sum += rxBuffer[cnt];
	}
	if (sum != rxBuffer[10]) {
		return false;
	}
	switch (rxBuffer[1])
	{
		case 0x50:  memcpy(&JY61P_data.time,    &rxBuffer[2], 8); break;    
		case 0x51:  memcpy(&JY61P_data.acc,     &rxBuffer[2], 8); break;    
		case 0x52:  memcpy(&JY61P_data.gyro,    &rxBuffer[2], 8); break;    
		case 0x53:  memcpy(&JY61P_data.angle,   &rxBuffer[2], 8); break;    
		case 0x54:  memcpy(&JY61P_data.mag,     &rxBuffer[2], 8); break;    
		case 0x55:  memcpy(&JY61P_data.dStatus, &rxBuffer[2], 8); break;    
		case 0x56:  memcpy(&JY61P_data.pressure,&rxBuffer[2], 4);           
		            memcpy(&JY61P_data.altitude,&rxBuffer[6], 4);           
		            break;
		case 0x57:  memcpy(&JY61P_data.lon,     &rxBuffer[2], 4);           
		            memcpy(&JY61P_data.lat,     &rxBuffer[6], 4);           
		            break;
		case 0x58:  memcpy(&JY61P_data.GPSHeight,   &rxBuffer[2], 2);       
		            memcpy(&JY61P_data.GPSYaw,      &rxBuffer[4], 2);
		            memcpy(&JY61P_data.GPSVelocity, &rxBuffer[6], 4);
		            break;
	}
	lastTime = millis();
	return true;
}

bool CJY61P::receiveSerialData(void)
{
	bool status = false;
	while (Serial_->available()) {
		status = CJY61P::readSerialData(Serial_->read());
	}
	return status;
}

void CJY61P::readData(uint8_t address, uint8_t length, uint8_t data[])
{
	readRegisters(address_, address, length, data);
}

uint16_t CJY61P::getTime(const char* str)
{
	if (transferMode_)
		readRegisters(address_, JY_YYMM, 8, (uint8_t*)&JY61P_data.time);

	if (strcmp(str, "year") == 0)       
		return JY61P_data.time.year;

	if (strcmp(str, "month") == 0)      
		return JY61P_data.time.month;

	if (strcmp(str, "day") == 0)        
		return JY61P_data.time.day;

	if (strcmp(str, "hour") == 0)       
		return JY61P_data.time.hour;

	if (strcmp(str, "minute") == 0)     
		return JY61P_data.time.minute;

	if (strcmp(str, "second") == 0)     
		return JY61P_data.time.second;

	if (strcmp(str, "milisecond") == 0) 
		return JY61P_data.time.milisecond;

	return 0;
}

double CJY61P::getAccX()
{
	if (transferMode_)
		readRegisters(address_, JY_AX, 2, (uint8_t *)&JY61P_data.acc.x);
	return JY61P_data.acc.x / (32768.0/16.0);
}

double CJY61P::getAccY()
{
	if (transferMode_)
		readRegisters(address_, JY_AY, 2, (uint8_t *)&JY61P_data.acc.y);
	return JY61P_data.acc.y / (32768.0/16.0);
}

double CJY61P::getAccZ()
{
	if (transferMode_)
		readRegisters(address_, JY_AZ, 2, (uint8_t *)&JY61P_data.acc.z);
	return JY61P_data.acc.z / (32768.0/16.0);
}

double CJY61P::getGyroX()
{
	if (transferMode_)
		readRegisters(address_, JY_GX, 2, (uint8_t *)&JY61P_data.gyro.x);
	return JY61P_data.gyro.x / (32768.0/2000.0);
}

double CJY61P::getGyroY()
{
	if (transferMode_)
		readRegisters(address_, JY_GY, 2, (uint8_t *)&JY61P_data.gyro.y);
	return JY61P_data.gyro.y / (32768.0/2000.0);
}

double CJY61P::getGyroZ()
{
	if (transferMode_)
		readRegisters(address_, JY_GZ, 2, (uint8_t *)&JY61P_data.gyro.z);
	return JY61P_data.gyro.z / (32768.0/2000.0);
}

double CJY61P::getMagX()
{
	if (transferMode_)
		readRegisters(address_, JY_HX, 2, (uint8_t *)&JY61P_data.mag.x);
	return JY61P_data.mag.x / (32768.0/180.0);
}

double CJY61P::getMagY()
{
	if (transferMode_)
		readRegisters(address_, JY_HY, 2, (uint8_t *)&JY61P_data.mag.y);
	return JY61P_data.mag.y / (32768.0/180.0);
}

double CJY61P::getMagZ()
{
	if (transferMode_)
		readRegisters(address_, JY_HZ, 2, (uint8_t *)&JY61P_data.mag.z);
	return JY61P_data.mag.z / (32768.0/180.0);
}

int16_t CJY61P::getAccRawX()
{
	if (transferMode_)
		readRegisters(address_, JY_AX, 2, (uint8_t *)&JY61P_data.acc.x);
	return JY61P_data.acc.x;
}

int16_t CJY61P::getAccRawY()
{
	if (transferMode_)
		readRegisters(address_, JY_AY, 2, (uint8_t *)&JY61P_data.acc.y);
	return JY61P_data.acc.y;
}

int16_t CJY61P::getAccRawZ()
{
	if (transferMode_)
		readRegisters(address_, JY_AZ, 2, (uint8_t *)&JY61P_data.acc.z);
	return JY61P_data.acc.z;
}

int16_t CJY61P::getGyroRawX()
{
	if (transferMode_)
		readRegisters(address_, JY_GX, 2, (uint8_t *)&JY61P_data.gyro.x);
	return JY61P_data.gyro.x;
}

int16_t CJY61P::getGyroRawY()
{
	if (transferMode_)
		readRegisters(address_, JY_GY, 2, (uint8_t *)&JY61P_data.gyro.y);
	return JY61P_data.gyro.y;
}

int16_t CJY61P::getGyroRawZ()
{
	if (transferMode_)
		readRegisters(address_, JY_GZ, 2, (uint8_t *)&JY61P_data.gyro.z);
	return JY61P_data.gyro.z;
}

int16_t CJY61P::getMagRawX()
{
	if (transferMode_)
		readRegisters(address_, JY_HX, 2, (uint8_t *)&JY61P_data.mag.x);
	return JY61P_data.mag.x;
}

int16_t CJY61P::getMagRawY()
{
	if (transferMode_)
		readRegisters(address_, JY_HY, 2, (uint8_t *)&JY61P_data.mag.y);
	return JY61P_data.mag.y;
}

int16_t CJY61P::getMagRawZ()
{
	if (transferMode_)
		readRegisters(address_, JY_HZ, 2, (uint8_t *)&JY61P_data.mag.z);
	return JY61P_data.mag.z;
}

double CJY61P::getRoll()
{
		if (transferMode_)
			readRegisters(address_, JY_Roll, 2, (uint8_t *)&JY61P_data.angle.roll);
		return JY61P_data.angle.roll / (32768.0/180.0);
}

double CJY61P::getPitch()
{
		if (transferMode_)
			readRegisters(address_, JY_Pitch, 2, (uint8_t *)&JY61P_data.angle.pitch);
		return JY61P_data.angle.pitch / (32768.0/180.0);
}
double CJY61P::getYaw()
{
		if (transferMode_)
			readRegisters(address_, JY_Yaw, 2, (uint8_t *)&JY61P_data.angle.yaw);
		return JY61P_data.angle.yaw / (32768.0/180.0);
}


int32_t CJY61P::getPressure(void)
{
	if (transferMode_)
		readRegisters(address_, JY_PressureL, 4, (uint8_t *)&JY61P_data.pressure);

	return JY61P_data.pressure; //Pa
}

int32_t CJY61P::getAltitude(void)
{
	if (transferMode_)
		readRegisters(address_, JY_HeightL, 4, (uint8_t *)&JY61P_data.altitude);

	return JY61P_data.altitude; //cm

}


int16_t CJY61P::getD0Status()
{
	if (transferMode_)
		readRegisters(address_, JY_D0Status, 2, (uint8_t *)&JY61P_data.dStatus.d_0);
		return JY61P_data.dStatus.d_0;
}

int16_t CJY61P::getD1Status()
{
	if (transferMode_)
		readRegisters(address_, JY_D1Status, 2, (uint8_t *)&JY61P_data.dStatus.d_1);
		return JY61P_data.dStatus.d_1;
}

int16_t CJY61P::getD2Status()
{
	if (transferMode_)
		readRegisters(address_, JY_D2Status, 2, (uint8_t *)&JY61P_data.dStatus.d_2);
		return JY61P_data.dStatus.d_2;
}

int16_t CJY61P::getD3Status()
{
	if (transferMode_)
		readRegisters(address_, JY_D3Status, 2, (uint8_t *)&JY61P_data.dStatus.d_3);
		return JY61P_data.dStatus.d_3;
}

int32_t CJY61P::getLon(void)
{
	if (transferMode_)
		readRegisters(address_, JY_LonL, 4, (uint8_t *)&JY61P_data.lon);

	return JY61P_data.lon;
}

int32_t CJY61P::getLat(void)
{
	if (transferMode_)
		readRegisters(address_, JY_LatL, 4, (uint8_t *)&JY61P_data.lat);

	return JY61P_data.lat;
}

double CJY61P::getGPSH(void)
{
	if (transferMode_)
		readRegisters(address_, JY_GPSHeight, 2, (uint8_t *)&JY61P_data.GPSHeight);

	return JY61P_data.GPSHeight / 10.0;
}

double CJY61P::getGPSY(void)    //度
{
	if (transferMode_)
		readRegisters(address_, JY_GPSYAW, 2, (uint8_t *)&JY61P_data.GPSYaw);

	return JY61P_data.GPSYaw / 10.0;
}

double CJY61P::getGPSV(void)    //km/h
{
	if (transferMode_)
		readRegisters(address_, JY_GPSVL, 4, (uint8_t *)&JY61P_data.GPSVelocity);

	return JY61P_data.GPSVelocity / 1000.0;
}

void CJY61P::saveConf(void)
{
	if (transferMode_) {
		uint8_t cmd[2] = {0x00,0x00};
		writeRegister(address_, JY_SAVE, 2, cmd);
	}
}

void CJY61P::quitCali(void)
{
	if (transferMode_) {
		uint8_t cmd[2] = {0x00,0x00};
		writeRegister(address_, JY_CALSW, 2, cmd);
	}
}

void CJY61P::caliIMU(void)
{
	if (transferMode_) {
		uint8_t cmd[2] = {0x01,0x00};
		writeRegister(address_, JY_CALSW, 2, cmd);
	}
}

void CJY61P::caliMag(void)
{
	if (transferMode_) {
		uint8_t cmd[2] = {0x02,0x00};
		writeRegister(address_, JY_CALSW, 2, cmd);
	}
}


unsigned long CJY61P::getLastTime(void)
{
	return lastTime;
}

void CJY61P::readRegisters(uint8_t deviceAddr, uint8_t addressToRead, uint8_t bytesToRead, uint8_t * dest)
{
	Wire.beginTransmission(deviceAddr);
	Wire.write(addressToRead);
	Wire.endTransmission(false); //endTransmission but keep the connection active

	Wire.requestFrom(deviceAddr, bytesToRead); //Ask for bytes, once done, bus is released by default

	if (Wire.available() >= bytesToRead) {//Hang out until we get the # of bytes we expect
		for (int x = 0; x < bytesToRead; x++)
			dest[x] = Wire.read();
		lastTime = millis();
	}
}

void CJY61P::writeRegister(uint8_t deviceAddr, uint8_t addressToWrite, uint8_t bytesToRead, uint8_t *dataToWrite)
{
	Wire.beginTransmission(deviceAddr);
	Wire.write(addressToWrite);
	for (int i = 0; i < bytesToRead; i++)
		Wire.write(dataToWrite[i]);
	Wire.endTransmission(); //Stop transmitting
}

CJY61P JY61P;
