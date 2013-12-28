///////////////////////////////////////////////////////////////////////////////////////////
//
// "2xSaI : The advanced 2x Scale and Interpolation engine" - written by Derek Liauw Kie Fa
//
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _2XSAI_H_
#define _2XSAI_H_

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

int Init_2xSaI(uint32);
void Super2xSaI(uint8*,uint32,uint8*,uint8*,uint32,int,int);
void SuperEagle(uint8*,uint32,uint8*,uint8*,uint32,int,int);
void _2xSaI(uint8*,uint32,uint8*,uint8*,uint32,int,int);

#endif
