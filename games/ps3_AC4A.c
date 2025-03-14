//===========================================================
// Mouse Injector for Dolphin
//==========================================================================
// Copyright (C) 2019-2020 Carnivorous
// All rights reserved.
//
// Mouse Injector is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, visit http://www.gnu.org/licenses/gpl-2.0.html
//==========================================================================
#include <stdint.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define PI 3.14159265f // 0x40490FDB
#define TAU 6.2831853f // 0x40C90FDB

#define AC4A_CAM_START_IDENTIFIER 0x0123A9B8 //i believe this hex identifies the start of the player cam, there are several copies of it in memory, but the only relevant one is the first one stored between 0x332000000 and 0x333000000
#define AC4A_CAMY_STRING 0x3D567750U //you can always find the camy float at this string location + 0x4. (after the cam start identifier) string should read "=VwP"

//FF FF FF FF 
//FF FF FF FF 
//00 00 00 00 
//00 00 00 05 
//00 00 00 00 
//00 00 00 05  this byte array is 0x68 behind the first sine value
//believe me i know how scuffed this is but until I find a real way of finding pointers in RPCS3 memory this is the best I got
#define AC4A_SIN_STRING1 0xFFFFFFFFU 
#define AC4A_SIN_STRING2 0xFFFFFFFFU 
#define AC4A_SIN_STRING3 0x00000000U
#define AC4A_SIN_STRING4 0x00000005U 
#define AC4A_SIN_STRING5 0x00000000U
#define AC4A_SIN_STRING6 0x00000005U

#define AC4A_CAM_SEARCH_MIN 0x2000000 //search values for finding the camera (I am very bad at finding pointers in the ps3)
#define AC4A_CAM_SEARCH_MAX 0x3000000 //if you have advice on finding them, please help

#define AC4A_CAMY 0x2DA6090 //not static, fix later
#define AC4A_SINE1 0x2D9B1E0 //NOT STATIC FIX LATER THESE TWO SHOULD REMAIN IDENTICAL
#define AC4A_COS1 0x2D9B1E8 //NOT STATIC FIX LATER THIS COSINE IS INVERSE
#define AC4A_COS2 0x2D9B200 //NOT STATIC FIX LATER THIS COSINE IS CORRECT
#define AC4A_SINE2 0x2D9B208 //NOT STATIC FIX LATER THESE TWO SHOULD REMAIN IDENTICAL

static uint8_t PS3_AC4A_Status(void);
static void PS3_AC4A_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Amored Core: For Answer",
	PS3_AC4A_Status,
	PS3_AC4A_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS3_AC4A = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS3_AC4A_Status(void)
{
  //41 69 52 65 
  //73 6F 75 00
	return (PS3_MEM_ReadUInt(0x1231) == 0x41695265U && 
          PS3_MEM_ReadUInt(0x1235) == 0x736F7500U);
}

static void AC4A_Find_Camera(int *camStart, int *camY, int *sine1)
{
  int camSearchIndex = AC4A_CAM_SEARCH_MIN;
  int camStartFound = 0;
  int camYFound = 0;
  int sine1Found = 0;
  while (camSearchIndex <= AC4A_CAM_SEARCH_MAX)
  {
    if(PS3_MEM_ReadUInt(camSearchIndex) == AC4A_CAM_START_IDENTIFIER && !camStartFound)
    {
      camStartFound = camSearchIndex;
      printf("Cam start found \n");
    }
    else if(PS3_MEM_ReadUInt(camSearchIndex) == AC4A_CAMY_STRING && 
            !camYFound && camStartFound)
    {
      camYFound = camSearchIndex;// + 0x4;
      printf("Camy found \n");
    }
    else if(PS3_MEM_ReadUInt(camSearchIndex) == AC4A_SIN_STRING1 &&
            PS3_MEM_ReadUInt(camSearchIndex + 0x4) == AC4A_SIN_STRING2 &&
            PS3_MEM_ReadUInt(camSearchIndex + 0x8) == AC4A_SIN_STRING3 &&
            PS3_MEM_ReadUInt(camSearchIndex + 0xC) == AC4A_SIN_STRING4 &&
            PS3_MEM_ReadUInt(camSearchIndex + 0x10) == AC4A_SIN_STRING5 &&
            PS3_MEM_ReadUInt(camSearchIndex + 0x14) == AC4A_SIN_STRING6 &&
            !sine1Found && camStartFound)
    {
      sine1Found = camSearchIndex;// + 0x68;
      printf("sine1Found found \n");
    }

    if (camStartFound && camYFound && sine1Found) 
    {
      printf("Addresses found\n");
      *camY = camYFound;
      *sine1 = sine1Found;
      *camStart = camStartFound;
      break;
    }

    camSearchIndex += 0x4;
    
  }
}

int camStartAddress;
int camYPrecursor;
int sine1Precursor;

int camYAddress;
int sine1Address;
int sine2Address;
int cosine1Address;
int cosine2Address;
static void PS3_AC4A_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
  
  float looksensitivity = (float)sensitivity;
  float scale = 12000.f;


  if (!(PS3_MEM_ReadUInt(camStartAddress) == AC4A_CAM_START_IDENTIFIER && //sanity check the memory locations
      PS3_MEM_ReadUInt(camYPrecursor) == AC4A_CAMY_STRING &&
     (PS3_MEM_ReadUInt(sine1Precursor) == AC4A_SIN_STRING1 &&
      PS3_MEM_ReadUInt(sine1Precursor + 0x4) == AC4A_SIN_STRING2 &&
      PS3_MEM_ReadUInt(sine1Precursor + 0x8) == AC4A_SIN_STRING3 &&
      PS3_MEM_ReadUInt(sine1Precursor + 0xC) == AC4A_SIN_STRING4 &&
      PS3_MEM_ReadUInt(sine1Precursor + 0x10) == AC4A_SIN_STRING5 &&
      PS3_MEM_ReadUInt(sine1Precursor + 0x14) == AC4A_SIN_STRING6)))
  {
    printf("looking for camera\n");
    AC4A_Find_Camera(&camStartAddress, &camYPrecursor, &sine1Precursor);
    camYAddress = camYPrecursor + 0x4;
    sine1Address = sine1Precursor + 0x68;
    cosine1Address = sine1Address + 0x8;
    cosine2Address = sine1Address + 0x20;
    sine2Address = cosine2Address + 0x8;
  }


  float camXSin =  PS3_MEM_ReadFloat(sine1Address);
  float camXCos = PS3_MEM_ReadFloat(cosine2Address);
  float camY = PS3_MEM_ReadFloat(camYAddress);

  float CamXAngle = atan(camXSin / camXCos);

  CamXAngle -= (float)xmouse * looksensitivity / scale;

  if (camXCos < 0)
		CamXAngle += PI;
/*while (camXCos < -PI)
		CamXAngle += TAU;*/
  

  camY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;
  camXCos = cos(CamXAngle);
  camXSin = sin(CamXAngle);


  while (camY > 1.221730471f)
    camY = 1.221730471f;
  while (camY < -1.221730471f)
    camY = -1.221730471f;



  PS3_MEM_WriteFloat(camYAddress, camY);
  PS3_MEM_WriteFloat(sine1Address, camXSin);
  PS3_MEM_WriteFloat(cosine1Address, -camXCos);
  PS3_MEM_WriteFloat(cosine2Address, camXCos);
  PS3_MEM_WriteFloat(sine2Address, camXSin);
}