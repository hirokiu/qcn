/*
 *  csensor_mac_usb_onavi01.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 10/25/2009.
 *  Copyright 2009 Stanford University.  All rights reserved.
 *
 * Implementation file for Mac ONavi 1 Serial Comunications
 */

#include "main.h"
#include "csensor_mac_usb_onavi01.h"
 
CSensorMacUSBONavi01::CSensorMacUSBONavi01()
  : CSensor()
{ 
}

CSensorMacUSBONavi01::~CSensorMacUSBONavi01()
{
  closePort();
}

void CSensorMacUSBONavi01::closePort()
{
	setPort();
	setType();
}

bool CSensorMacUSBONavi01::detect()
{
/*
					dcb.BaudRate = CBR_115200;
					dcb.ByteSize = 8;
					dcb.Parity = NOPARITY;
					dcb.StopBits = ONESTOPBIT;
					dcb.fBinary = TRUE;
					dcb.fOutxCtsFlow = FALSE;
					dcb.fOutxDsrFlow = FALSE;
					dcb.fDtrControl = DTR_CONTROL_DISABLE;
*/					
	return bRet;
}

inline bool CSensorMacUSBONavi01::read_xyz(float& x1, float& y1, float& z1)
{
	/*
We tried to keep the data as simple as possible. The data looks like: 

##xxyyzzs 

Two ASCII '#' (x23) followed by the X value upper byte, X value lower byte, Y value upper byte, Y value lower byte, Z value upper byte, Z value lower byte and an eight bit checksum.  

The bytes are tightly packed and there is nothing between the data values except for the sentence start ##.  

The X and Y axis will have a 0g offset at 32768d (x8000) and the Z axis offset at +1g 45874d (xB332) when oriented in the X/Y horizontal and Z up position.  The  s  value is a one byte checksum.  

It is a sum of all of the bytes, truncated to the lower byte.  This firmware does not transmit the temperature value. 

Finding g as a value:

g  = x - 32768 * (5 / 65536) 

Where: x is the data value 0 - 65536 (x0000 to xFFFF). 

Values >32768 are positive g and <32768 are negative g. The sampling rate is set to 200Hz, and the analog low-pass filters are set to 50Hz.  The data is oversampled 2X over Nyquist. We are going to make a new version of the module, with 25Hz LP analog filters and dual sensitivity 2g / 6g shortly.  Same drivers, same interface.  I ll get you one as soon as I we get feedback on this and make a set of those.

	*/

	// first check for valid port
	if (getPort() < 0) return false;

	bool bRet = false;

//	bRet = (bool) ::ReadFile(m_hcom, bytesIn, 32, &dwRead, NULL);

	if (bRet) {
		for (int i = 31; i > 0; i--) { // look for hash-mark i.e. ## boundaries (two sets of ##)
			if (bytesIn[i] == 0x23 && bytesIn[i-1] == 0x23) { // found a hash-mark set
				if (!lOffset[1]) lOffset[1] = i-1;
				else {
					lOffset[0] = i; // must be the start
					break;  // found both hash marks - can leave loop
				}
			}
 		}
		if (lOffset[0] && lOffset[1] && lOffset[1] == (lOffset[0] + 8)) { 
                   // we found both, the bytes in between are what we want (really bytes after lOffset[0]
			x = (bytesIn[lOffset[0]+1] * 255) + bytesIn[lOffset[0]+2];
			y = (bytesIn[lOffset[0]+3] * 255) + bytesIn[lOffset[0]+4];
			z = (bytesIn[lOffset[0]+5] * 255) + bytesIn[lOffset[0]+6];
			cs   = bytesIn[lOffset[0]+7];
			for (int i = 1; i <= 6; i++) iCS += bytesIn[lOffset[0] + i];

			// convert to g decimal value
			// g  = x - 32768 * (5 / 65536) 
			// Where: x is the data value 0 - 65536 (x0000 to xFFFF). 

			x1 = ((float) x - 32768.0f) * FLOAT_ONAVI_FACTOR;
			y1 = ((float) y - 32768.0f) * FLOAT_ONAVI_FACTOR;
			z1 = ((float) z - 32768.0f) * FLOAT_ONAVI_FACTOR;
		}
		else {
			bRet = false;
		}
	}

	return bRet;
}


