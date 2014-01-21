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

#include <android/input.h>
#include <android/sensor.h>

// this is the Linux implementation for the JoyWarrior sensor, used for QCNLive as well as the Mac service program qcnmacusb under BOINC
class CSensorAndroidBuiltIn  : public CSensor
{
   private:
      // vars lifted from the codemercs.com JW24F8 Linux example
      int m_fdJoy, *m_piAxes, m_iNumAxes, m_iNumButtons;
      char *m_strButton, m_strJoystick[80];

      virtual bool read_xyz(float& x1, float& y1, float& z1);  

      bool testJoystick();  // tests that it really is the JoyWarrior & sets to "raw data" mode

   public:
      CSensorAndroidBuiltIn();
      virtual ~CSensorAndroidBuiltIn();

      virtual bool detect();    // this detects the Mac USB sensor
      virtual void closePort(); // closes the port if open

};

#endif   // ANDROID

#endif
