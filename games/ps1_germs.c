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

#define TAU 6.2831853f // 0x40C90FDB

#define GERMS_CAMY 0x1E068C
#define GERMS_CAMX 0x1E068E
#define GERMS_TRAIN_CAMY 0x1E114C
#define GERMS_TRAIN_CAMX 0x1E114E
#define GERMS_CAR_CAMY 0x1E117C
#define GERMS_CAR_CAMX 0x1E117E

static uint8_t PS1_GERMS_Status(void);
static void PS1_GERMS_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Germs : Nerawareta Machi\n Make sure to use the required cheat file for this game.",
	PS1_GERMS_Status,
	PS1_GERMS_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS1_GERMS = &GAMEDRIVER_INTERFACE;

static float xAccumulator = 0.f;
static float yAccumulator = 0.f;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS1_GERMS_Status(void)
{
  //534C5053
  //5F303231
  //2E30373B

	return (PS1_MEM_ReadWord(0x92EC) == 0x534C5053U && PS1_MEM_ReadWord(0x92F0) == 0x5F303231U && PS1_MEM_ReadWord(0x92F4) == 0x2E30373BU);
}

static void PS1_GERMS_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	
  uint16_t camX = PS1_MEM_ReadHalfword(GERMS_CAMX);
	uint16_t camY = PS1_MEM_ReadHalfword(GERMS_CAMY);

  uint16_t trainCamX = PS1_MEM_ReadHalfword(GERMS_TRAIN_CAMX);
	uint16_t trainCamY = PS1_MEM_ReadHalfword(GERMS_TRAIN_CAMY);

  uint16_t carCamX = PS1_MEM_ReadHalfword(GERMS_CAR_CAMX);
	uint16_t carCamY = PS1_MEM_ReadHalfword(GERMS_CAR_CAMY);

	float camXF = (float)camX;
	float camYF = (float)camY;

  float trainCamXF = (float)trainCamX;
	float trainCamYF = (float)trainCamY;

  float carCamXF = (float)carCamX;
	float carCamYF = (float)carCamY;

	const float Yscale = .1f;
  const float Xscale = .1f;
	const float looksensitivity = (float)sensitivity / 20.f;

  //NORMAL LOOK
	float dx = (float)xmouse * looksensitivity * Xscale;
	AccumulateAddRemainder(&camXF, &xAccumulator, xmouse, dx);

	float ym = -(float)(invertpitch ? -ymouse : ymouse);
	float dy = ym * looksensitivity * Yscale;
	AccumulateAddRemainder(&camYF, &yAccumulator, ym, dy);

  //TRAIN LOOK
  float trainDX = (float)xmouse * looksensitivity * Xscale;
	AccumulateAddRemainder(&trainCamXF, &xAccumulator, xmouse, trainDX);

	float trainDY = ym * looksensitivity * Yscale;
	AccumulateAddRemainder(&trainCamYF, &yAccumulator, ym, trainDY);

  //CAR LOOK
  float carDX = (float)xmouse * looksensitivity * Xscale;
	AccumulateAddRemainder(&carCamXF, &xAccumulator, xmouse, carDX);

	float carDY = ym * looksensitivity * Yscale;
	AccumulateAddRemainder(&carCamYF, &yAccumulator, ym, carDY);

	// clamp y-axis
	// range is larger than game usually allows but feels better
	if (camYF > 65 && camYF < 32000) 
  camYF = 65;
	
  
  if (camYF < 65511 && camYF > 32000) 
  camYF = 65511;

  /*if (camXF < 0)
  camXF = 360 - camXF;
  if (camXF > 359)
  camXF = 0 + camXF;*/

  //clamp train view rotations
  if (trainCamYF > 65 && trainCamYF < 32000) 
  trainCamYF = 65;
	
  
  if (trainCamYF < 65511 && trainCamYF > 32000) 
  trainCamYF = 65511;

  //clamp car view rotations
  if (carCamXF > 65 && carCamXF < 32000)
  carCamXF = 65;
  if (carCamXF < 65471 && carCamXF > 32000)
  carCamXF = 65471;

  if (carCamYF > 20 && carCamYF < 32000)
  carCamYF = 20;
  if (carCamYF < 65516 && carCamYF > 32000)
  carCamYF = 65516;
  
  
	PS1_MEM_WriteHalfword(GERMS_CAMX, (uint16_t)camXF);
	PS1_MEM_WriteHalfword(GERMS_CAMY, (uint16_t)camYF);
  PS1_MEM_WriteHalfword(GERMS_TRAIN_CAMX, (uint16_t)trainCamXF);
	PS1_MEM_WriteHalfword(GERMS_TRAIN_CAMY, (uint16_t)trainCamYF);
  PS1_MEM_WriteHalfword(GERMS_CAR_CAMX, (uint16_t)carCamXF);
	PS1_MEM_WriteHalfword(GERMS_CAR_CAMY, (uint16_t)carCamYF);
}