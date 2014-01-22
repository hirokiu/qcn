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

ASensorEventQueue* l_pSensorEventQueue = NULL;

/*
int QCN_ALooper_callback(int fd, int events, void* data)
{
  return 0;
  ASensorEvent event;
  while (ASensorEventQueue_getEvents(l_pSensorEventQueue, &event, 1) > 0) {
     if(event.type==ASENSOR_TYPE_ACCELEROMETER) {
            fprintf(stdout, "callback: accl(x,y,z,t): %f %f %f %lld\n",
                 event.acceleration.x, event.acceleration.y,
                 event.acceleration.z, event.timestamp);
     }
   }
  return 1;  // return 1 to continue receiving callbacks
}
*/

CSensorAndroidBuiltIn::CSensorAndroidBuiltIn()
  : CSensor(), m_pSensorManager(NULL), m_pSensor(NULL), m_pLooper(NULL), 
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
  if (m_pSensor && l_pSensorEventQueue) {
     ASensorEventQueue_disableSensor(l_pSensorEventQueue, m_pSensor);
  }
  if (m_pSensorManager && l_pSensorEventQueue) {
     ASensorManager_destroyEventQueue(m_pSensorManager, l_pSensorEventQueue);
  }

  strcpy(m_strVendor, "");
  strcpy(m_strSensor, "");

  m_pSensorManager = NULL;
  m_pSensor = NULL;
  l_pSensorEventQueue = NULL;
  m_pLooper = NULL;

  m_fResolution = 0.0f;
  m_minDelayMsec = 0;

  memset(m_strSensor, 0x00, _MAX_PATH);
  memset(m_strVendor, 0x00, _MAX_PATH);

  setType();
  setPort();

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
    if (! m_pSensor || ! m_pLooper | ! l_pSensorEventQueue) return false;  // no open file descriptor

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

      int ident, events;
      float fCtr = 0.0;
      while((ident = ALooper_pollAll(-1, NULL, &events, NULL)) >= 0) {
         if (ident == LOOPER_ID_QCN) {
            ASensorEvent event;
            while (ASensorEventQueue_getEvents(l_pSensorEventQueue, &event, 1) > 0) {
              fCtr = fCtr + 1.0;
              x1 += event.acceleration.x;
              y1 += event.acceleration.y;
              z1 += event.acceleration.z;
            }
         }
      }
      if (fCtr > 0.0) {
          x1 = x1 / fCtr;
          y1 = y1 / fCtr;
          z1 = z1 / fCtr;
      }

    fprintf(stdout, "read_xyz:  %f %f %f\n", x1, y1, z1);
    return true;
}

// try and open up the JoyWarrior file descriptor
bool CSensorAndroidBuiltIn::detect()
{
   setType();
   setPort();

   // get the singleton instance of the m_pSensorManager
   if (!m_pSensorManager) m_pSensorManager = (ASensorManager*) ASensorManager_getInstance();

/*
   // sensor listing
   ASensorList pSensorList = NULL;
   int iNum = ASensorManager_getSensorList(m_pSensorManager, &pSensorList);
   if (iNum && pSensorList) {
      fprintf(stdout, "\n\n%d Detected Sensors:\n", iNum);
      //int i = 0;
      for (int i=0; i < iNum; i++) {
      //while (i<10 && (pSensorList+i)) { //for (int i=0; i < iNum; i++) {
          fprintf(stdout, "  %s\n", ASensor_getName(*(pSensorList+i)));
          i++;
      }
   }
   else {
      fprintf(stdout, "\n\nNo Sensor List?  %d\n\n", iNum);
   }
*/

   // get the default accelerometer
   m_pSensor = (ASensor*) ASensorManager_getDefaultSensor(m_pSensorManager, ASENSOR_TYPE_ACCELEROMETER);

   if (!m_pSensor) {
      //fprintf(stdout, "No Android accelerometer detected.\n");
      return false; // no sensor
   }


   m_fResolution = ASensor_getResolution(m_pSensor);
   m_minDelayMsec = ASensor_getMinDelay(m_pSensor);

   strlcpy(m_strSensor, ASensor_getName(m_pSensor), _MAX_PATH);
   strlcpy(m_strVendor, ASensor_getVendor(m_pSensor), _MAX_PATH);

   // create looper
   m_pLooper = ALooper_forThread(); // get existing looper
   if (!m_pLooper) {  // make new looper
     //m_pLooper = ALooper_prepare(0); //ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
     m_pLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
   }
   if (!m_pLooper) { // no existing or new looper -- error
     fprintf(stderr, "Android Default Sensor Detected - but can't create Looper: \n\n   %s - %s\n     Res = %f --- Min Delay msec = %d\n",
           m_strVendor, m_strSensor, m_fResolution, m_minDelayMsec);
      return false; // can't create looper
   }

   // setup event queue
   //l_pSensorEventQueue = ASensorManager_createEventQueue(m_pSensorManager, m_pLooper, LOOPER_ID_QCN, QCN_ALooper_callback, &m_SensorVector);
   l_pSensorEventQueue = ASensorManager_createEventQueue(m_pSensorManager, m_pLooper, LOOPER_ID_QCN, NULL, NULL);
   if (!l_pSensorEventQueue) {
     fprintf(stderr, "Android Default Sensor Detected - but can't create SensorEventQueue: \n\n   %s - %s\n     Res = %f --- Min Delay msec = %d\n",
           m_strVendor, m_strSensor, m_fResolution, m_minDelayMsec);
      return false;  // can't setup queue
   }

   ASensorEventQueue_enableSensor(l_pSensorEventQueue, m_pSensor);
   ASensorEventQueue_setEventRate(l_pSensorEventQueue, m_pSensor, (int)((sm->dt > 0. ? sm->dt : g_DT) * 1000.));

   fprintf(stdout, "Android Default Sensor Detected: \n\n   %s - %s\n"
                   "  Res = %f --- Min Delay msec = %d\n"
                   "  m_pSensor=%x    l_pSensorEventQueue=%x\n",
           m_strVendor, m_strSensor, m_fResolution, m_minDelayMsec, m_pSensor, l_pSensorEventQueue);

   setType(SENSOR_ANDROID);

#ifdef QCN_RAW_DATA
   setSingleSampleDT(true); // set to true in raw mode so we don't get any interpolated/avg points (i.e. just the "integer" value hopefully)
#else
   setSingleSampleDT(false);
#endif
   return (bool)(getTypeEnum() == SENSOR_ANDROID);
}

#endif // ANDROID

