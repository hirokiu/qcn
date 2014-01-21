/*
 *  csensor_android_built_in.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 01/14/2014.
 *  Copyright 2014 Stanford University.  All rights reserved.
 *
 * Implementation file for Linux JoyWarrior 24F8 USB sensor - 
 *    note this just does a simple read of the joystick, so no setting of sample rate etc on-board the JW24F8 (which is fine)
 */

#include "main.h"
#include "csensor_android_built_in.h"

#ifdef ANDROID

CSensorAndroidBuiltIn::CSensorAndroidBuiltIn()
  : CSensor(), m_pSensorManager(NULL), m_pSensor(NULL), m_pSensorEventQueue(NULL), m_pLooper(NULL), 
       m_fResolution(0.0f), m_minDelayMsec(0)
{ 
  memset(m_strSensor, 0x00, _MAX_PATH);
  memset(m_strVendor, 0x00, _MAX_PATH);
}

CSensorAndroidBuiltIn::~CSensorAndroidBuiltIn()
{
  closePort();
}

void CSensorAndroidBuiltIn::closePort()
{
  if (m_pSensorEventQueue) {
     ASensorEventQueue_disableSensor(m_pSensorEventQueue, m_pSensor);
  }
  setType();
  setPort();

  strcpy(m_strVendor, "");
  strcpy(m_strSensor, "");

  m_pSensorManager = NULL;
  m_pSensor = NULL;
  m_pSensorEventQueue = NULL;
  m_pLooper = NULL;

  m_fResolution = 0.0f;
  m_minDelayMsec = 0;

  memset(m_strSensor, 0x00, _MAX_PATH);
  memset(m_strVendor, 0x00, _MAX_PATH);
}


/*
ASENSOR_TYPE_ACCELEROMETER

int ASensor_getType(ASensor const* sensor);

float ASensor_getResolution(ASensor const* sensor) __NDK_FPABI__;

int ASensor_getMinDelay(ASensor const* sensor);

int ASensorEventQueue_enableSensor(ASensorEventQueue* queue, ASensor const* sensor);

int ASensorEventQueue_disableSensor(ASensorEventQueue* queue, ASensor const* sensor);

 * Returns this sensor's name (non localized)
const char* ASensor_getName(ASensor const* sensor);

 * Returns this sensor's vendor's name (non localized)
const char* ASensor_getVendor(ASensor const* sensor);

 * Return this sensor's type
int ASensor_getType(ASensor const* sensor);

 * Returns this sensors's resolution
float ASensor_getResolution(ASensor const* sensor) __NDK_FPABI__;

 * Returns the minimum delay allowed between events in microseconds.
 * A value of zero means that this sensor doesn't report events at a
 * constant rate, but rather only when a new data is available.
int ASensor_getMinDelay(ASensor const* sensor);

*/


inline bool CSensorAndroidBuiltIn::read_xyz(float& x1, float& y1, float& z1)
{  
#ifndef QCN_USB
    if (qcn_main::g_iStop) return false;
#endif
    if (! m_pSensor) return false;  // no open file descriptor

    // read the joystick state - range on each axis seems to be 0-1023 (-2 to 2g)
    x1 = y1 = z1 = 0.0f;

    // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
/*
#ifdef QCN_RAW_DATA
    x1 = (float) m_piAxes[0];
    y1 = (float) m_piAxes[1];
    z1 = (float) m_piAxes[2];
#else           
    x1 = EARTH_G * (((float) m_piAxes[0] - 512.0f ) / 256.0f);
    y1 = EARTH_G * (((float) m_piAxes[1] - 512.0f ) / 256.0f);
    z1 = EARTH_G * (((float) m_piAxes[2] - 512.0f ) / 256.0f);
#endif 
*/
    // fprintf(stderr, "x1 = %f   y1 = %f   z1 = %f\n", x1, y1, z1);

    return true;
}

// try and open up the JoyWarrior file descriptor
bool CSensorAndroidBuiltIn::detect()
{
   setType();
   setPort();

   // get the singleton instance of the m_pSensorManager
   if (!m_pSensorManager) m_pSensorManager = (ASensorManager*) ASensorManager_getInstance();

   // get the default accelerometer
   m_pSensor = (ASensor*) ASensorManager_getDefaultSensor(m_pSensorManager, ASENSOR_TYPE_ACCELEROMETER);

   if (!m_pSensor) {
      fprintf(stdout, "No Android accelerometer detected.\n");
      return false; // no sensor
   }

   setType(SENSOR_ANDROID);

   m_fResolution = ASensor_getResolution(m_pSensor);
   m_minDelayMsec = ASensor_getMinDelay(m_pSensor);

   strlcpy(m_strSensor, ASensor_getName(m_pSensor), _MAX_PATH);
   strlcpy(m_strVendor, ASensor_getVendor(m_pSensor), _MAX_PATH);

#ifdef QCN_RAW_DATA
   setSingleSampleDT(true); // set to true in raw mode so we don't get any interpolated/avg points (i.e. just the "integer" value hopefully)
#else
   setSingleSampleDT(false);
#endif
   return (bool)(getTypeEnum() == SENSOR_ANDROID);
}

#endif // ANDROID

