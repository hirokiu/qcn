#ifndef _CSENSOR_ANDROID_BUILT_IN_H_
#define _CSENSOR_ANDROID_BUILT_IN_H_

/*
 *  csensor_android_built_in.h
 *  qcn
 *
 *  Created by Carl Christensen on 01/14/2014.
 *  Copyright 2014 Stanford University
 *
 * This file contains the definition of the Android Device Built-In Accelerometer Sensor Class
 */

#include "main.h"
using namespace std;

#include <stdio.h>
#include <stdlib.h>

#ifdef ANDROID

#define LOOPER_QCN 100

#include <android/input.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>

// callback function for Android accelerometer events
int QCN_ALooper_callback(int fd, int events, void* data);

// this is the Linux implementation for the JoyWarrior sensor, used for QCNLive as well as the Mac service program qcnmacusb under BOINC
class CSensorAndroidBuiltIn  : public CSensor
{
   private:
      ASensorVector m_SensorVector;  // Android sensor vector for x/y/z readings etc
      ASensorManager* m_pSensorManager;  
      ASensor* m_pSensor;
      ASensorEventQueue* m_pSensorEventQueue;
      ALooper* m_pLooper;

      virtual bool read_xyz(float& x1, float& y1, float& z1);  

      char m_strSensor[_MAX_PATH];
      char m_strVendor[_MAX_PATH];

      float m_fResolution;
      int m_minDelayMsec;

   public:
      CSensorAndroidBuiltIn();
      virtual ~CSensorAndroidBuiltIn();

      virtual bool detect();    // this detects the Mac USB sensor
      virtual void closePort(); // closes the port if open

};

#endif   // ANDROID

#endif
