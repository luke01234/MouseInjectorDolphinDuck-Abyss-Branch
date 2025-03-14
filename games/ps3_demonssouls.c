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

static uint8_t PS3_DEMONSSOULS_Status(void);
static void PS3_DEMONSSOULS_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Demon's Souls",
	PS3_DEMONSSOULS_Status,
	PS3_DEMONSSOULS_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS3_DEMONSSOULS = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS3_DEMONSSOULS_Status(void)
{
  //42 4C 55 53 
  //33 30 34 34 
  //33 44 45 4D 
  //4F 4E 53 53
  //printf("PS3 Read %d\n", PS3_MEM_ReadUInt(0x8A9FF00));
	return (PS3_MEM_ReadUInt(0x8A9FF00) == 0x424C5553U && 
          PS3_MEM_ReadUInt(0x8A9FF04) == 0x33303434U && 
          PS3_MEM_ReadUInt(0x8A9FF08) == 0x3344454DU &&
          PS3_MEM_ReadUInt(0x8A9FF0C) == 0x4F4E5353U);
}

static void PS3_DEMONSSOULS_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	
}