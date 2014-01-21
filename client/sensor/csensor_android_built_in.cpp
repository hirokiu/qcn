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
  : CSensor(), m_fdJoy(-1), m_piAxes(NULL), m_iNumAxes(0), m_iNumButtons(0), m_strButton(NULL)
{ 
   // vars lifted from the codemercs.com JW24F8 Linux example
   memset(m_strJoystick, 0x00, 80);
}

CSensorAndroidBuiltIn::~CSensorAndroidBuiltIn()
{
  closePort();
}

void CSensorAndroidBuiltIn::closePort()
{
  if (m_fdJoy > -1) {
     close(m_fdJoy);
     m_fdJoy = -1;
  }
  if (m_piAxes) {
     free(m_piAxes);
     m_piAxes = NULL;
  }
  if (m_strButton) {
     free(m_strButton);
     m_strButton = NULL;
  }

  if (getPort() > -1) { // nothing really left to close, as it's just the joystick #
    fprintf(stdout, "Joywarrior 24F8 closed on Linux joystick port!\n");
    fflush(stdout);
  }
  setType();
  setPort();
}

inline bool CSensorAndroidBuiltIn::read_xyz(float& x1, float& y1, float& z1)
{  
#ifndef QCN_USB
    if (qcn_main::g_iStop) return false;
#endif
    if (m_fdJoy == -1) return false;  // no open file descriptor

    // read the joystick state - range on each axis seems to be 0-1023 (-2 to 2g)
    x1 = y1 = z1 = 0.0f;

    // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
#ifdef QCN_RAW_DATA
    x1 = (float) m_piAxes[0];
    y1 = (float) m_piAxes[1];
    z1 = (float) m_piAxes[2];
#else           
    x1 = EARTH_G * (((float) m_piAxes[0] - 512.0f ) / 256.0f);
    y1 = EARTH_G * (((float) m_piAxes[1] - 512.0f ) / 256.0f);
    z1 = EARTH_G * (((float) m_piAxes[2] - 512.0f ) / 256.0f);
#endif 

    // fprintf(stderr, "x1 = %f   y1 = %f   z1 = %f\n", x1, y1, z1);

    return true;
}

// tests the open joystick file descriptor to see if it's really a JW, and set to read rawdata
bool CSensorAndroidBuiltIn::testJoystick()
{
   // if made it here, then we have opened a joystick file descriptor
   m_iNumAxes = 0;
   m_iNumButtons = 0;
   memset(m_strJoystick, 0x00, 80);

//fprintf(stdout, "joystick found = %s\n", m_strJoystick);
//fflush(stdout);

   // compare the name of device, and number of buttons & axes with valid JoyWarrior values
   if (strcmp(m_strJoystick, IDSTR_JW24F8)
     || m_iNumButtons != NUM_BUTTON_JW24F8
     || m_iNumAxes != NUM_AXES_JW24F8) {
         closePort();  // this far in, we need to close the port!
         return false;
   }

   m_piAxes = (int *) calloc( m_iNumAxes, sizeof( int ) );
   memset(m_piAxes, 0x00, sizeof(int) * m_iNumAxes);
   m_strButton = (char *) calloc( m_iNumButtons, sizeof( char ) );
   memset(m_strButton, 0x00, sizeof(char) * m_iNumButtons);
  
   fcntl( m_fdJoy, F_SETFL, O_NONBLOCK );   // use non-blocking mode

   // try a read
   float x,y,z;
   // "prime" the joystick reader
   if (! read_xyz(x,y,z)) {
      closePort();  // this far in, we need to close the port!
      return false;
   }

   setType(SENSOR_ANDROID);
   setPort(getTypeEnum());
   
   return true; // if here we can return true, i.e Joywarrior found on Linux joystick port, and hopefully set to read raw data
}

// try and open up the JoyWarrior file descriptor
bool CSensorAndroidBuiltIn::detect()
{
   setType();
   setPort();

   const char* strJWEnum[LINUX_JOYSTICK_NUM] = LINUX_JOYSTICK_ARRAY;
   // go through and try potential Linux joystick devices from define.h
   int i;
   for (i = 0; i < LINUX_JOYSTICK_NUM; i++) {
      if (! boinc_file_exists(strJWEnum[i]) ) continue; // first see if file (device) exists
      if ( ( m_fdJoy = open(strJWEnum[i], O_RDONLY)) != -1 && testJoystick() ) {
         fprintf(stdout, "%s detected on joystick device %s\n", m_strJoystick, strJWEnum[i]);
         break;  // found a JW24F8
      }
   }
#ifdef QCN_RAW_DATA
   setSingleSampleDT(true); // set to true in raw mode so we don't get any interpolated/avg points (i.e. just the "integer" value hopefully)
#else
   setSingleSampleDT(false);
#endif
   return (bool)(getTypeEnum() == SENSOR_ANDROID);
}

#endif // ANDROID

