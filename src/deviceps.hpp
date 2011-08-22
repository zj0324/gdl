/* *************************************************************************
                          deviceps.hpp  -  postscript device
                             -------------------
    begin                : July 22 2002
    copyright            : (C) 2002 by Marc Schellens
    email                : m_schellens@users.sf.net
 ***************************************************************************/

/* *************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DEVICEPS_HPP_
#define DEVICEPS_HPP_

//#include "dstructgdl.hpp"
#include "gdlpsstream.hpp"
#include "initsysvar.hpp"

class DevicePS: public Graphics
{
  std::string      fileName;
  GDLPSStream*     actStream;
  float            XPageSize;
  float            YPageSize;
  int              color;
  int              decomposed; // false -> use color table

  void InitStream()
  {
    delete actStream;

    DLongGDL* pMulti = SysVar::GetPMulti();
    DLong nx = (*pMulti)[ 1];
    DLong ny = (*pMulti)[ 2];

    if( nx <= 0) nx = 1;
    if( ny <= 0) ny = 1;

    actStream = new GDLPSStream( nx, ny);

    actStream->sfnam( fileName.c_str());

    PLFLT xp; PLFLT yp; 
    PLINT xleng; PLINT yleng;
    PLINT xoff; PLINT yoff;
    actStream->gpage( xp, yp, xleng, yleng, xoff, yoff);
    
    //default xleng,yleng: 720,540
    //default in cm: 16.5,12.6
    float a; float scale;
    if (XPageSize != 0. && YPageSize != 0.) {a=XPageSize/YPageSize; scale=XPageSize/16.5;} 
    if (XPageSize == 0. && YPageSize == 0.) {a=540/720; scale=1.;} // default aspect ratio
    if (XPageSize == 0. && YPageSize != 0.) {a=540/720/(YPageSize/12.6); scale=1.;}
    if (XPageSize != 0. && YPageSize == 0.) {a=XPageSize/16.5*540/720; scale=1.;}
    char as[32];
    sprintf(as, "%f",a);
    actStream->setopt( "a", as); // this necessary to keep labels from looking stretched (plplot bug)
                                 // but plrender -a is also buggy: aspect ratios are not exactly correct 
    xleng=static_cast<PLINT>(floor(scale*540. +0.5));
    yleng=static_cast<PLINT>(floor(scale*720. +0.5));
    // setting this without plrender -a makes the labels stretched (plplot bug)
    actStream->spage( xp, yp, xleng, yleng, xoff, yoff); 

    actStream->setopt( "ori","1"); // portrait (upright)

    // no pause on destruction
    actStream->spause( false);

    // extended fonts
    actStream->fontld( 1);

    // set color map
    PLINT r[ctSize], g[ctSize], b[ctSize];
    actCT.Get( r, g, b);
    //    actStream->scmap0( r, g, b, ctSize); 
    actStream->scmap1( r, g, b, ctSize); 
    actStream->scolbg(255,255,255); // white background

    // default: black+white (IDL behaviour)
    //actStream->scolor( color); // has no effect
    if (color == 0) { actStream->setopt( "drvopt","text=0,color=0"); } 
    else { actStream->setopt( "drvopt","text=0,color=1");}
    color=0;

    actStream->Init();
    
    // load font
    actStream->font( 1);
    actStream->DefaultCharSize();

    //    (*pMulti)[ 0] = 0;
    actStream->adv(0);
  }

public:
  DevicePS(): Graphics(), fileName( "gdl.ps"), actStream( NULL), XPageSize(0.), YPageSize(0.), color(0), decomposed( 0)
  {
    name = "PS";

    DLongGDL origin( dimension( 2));
    DLongGDL zoom( dimension( 2));
    zoom[0] = 1;
    zoom[1] = 1;

    dStruct = new DStructGDL( "!DEVICE");
    dStruct->InitTag("NAME",       DStringGDL( name)); 
    dStruct->InitTag("X_SIZE",     DLongGDL( 17780)); 
    dStruct->InitTag("Y_SIZE",     DLongGDL( 12700)); 
    dStruct->InitTag("X_VSIZE",    DLongGDL( 640)); 
    dStruct->InitTag("Y_VSIZE",    DLongGDL( 512)); 
    dStruct->InitTag("X_CH_SIZE",  DLongGDL( 0)); 
    dStruct->InitTag("Y_CH_SIZE",  DLongGDL( 0)); 
    dStruct->InitTag("X_PX_CM",    DFloatGDL( 1000.0)); 
    dStruct->InitTag("Y_PX_CM",    DFloatGDL( 1000.0)); 
    dStruct->InitTag("N_COLORS",   DLongGDL( 256)); 
    dStruct->InitTag("TABLE_SIZE", DLongGDL( 256)); 
    dStruct->InitTag("FILL_DIST",  DLongGDL( 0)); 
    dStruct->InitTag("WINDOW",     DLongGDL( -1)); 
    dStruct->InitTag("UNIT",       DLongGDL( 0)); 
    dStruct->InitTag("FLAGS",      DLongGDL( 266807)); 
    dStruct->InitTag("ORIGIN",     origin); 
    dStruct->InitTag("ZOOM",       zoom); 
  }
  
  ~DevicePS()
  {
    delete actStream;
  }

  GDLGStream* GetStream( bool open=true)
  {
    if( actStream == NULL) 
      {
	if( !open) return NULL;
	InitStream();
      }
    return actStream;
  }

  bool SetFileName( const std::string& f)
  {
    fileName = f;
    return true;
  }

  bool CloseFile()
  {
    delete actStream;
    actStream = NULL;
    return true;
  }

  bool SetXPageSize( const float xs) // xs [cm]
  {
    XPageSize=xs;

    (*static_cast<DLongGDL*>(dStruct->GetTag(dStruct->Desc()->TagIndex("X_SIZE"))))[0] 
      = DLong(floor(0.5+
        xs * (*static_cast<DFloatGDL*>(dStruct->GetTag(dStruct->Desc()->TagIndex("X_PX_CM"))))[0]
      ));

    return true;
  }
  bool SetYPageSize( const float ys) // ys [cm]
  {
    YPageSize=ys;

    (*static_cast<DLongGDL*>(dStruct->GetTag(dStruct->Desc()->TagIndex("Y_SIZE"))))[0] 
      = DLong(floor(0.5+
        ys * (*static_cast<DFloatGDL*>(dStruct->GetTag(dStruct->Desc()->TagIndex("Y_PX_CM"))))[0]
      ));

    return true;
  }

  bool SetColor()
  {
    color=1;
    return true;
  }

  bool Decomposed( bool value)           
  {   
    decomposed = value;
    return true;
  }

  DLong GetDecomposed()        
  {
    return decomposed;  
  }
};

#endif
