/*
 * image.c: image handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#define X_DISPLAY_MISSING
#include <Magick++.h>

#include "image.h"
#include "common.h"
#include <sys/time.h> 

//#define DEBUG_IMAGETIMES

using namespace std; //required by exception handling
using namespace Magick;

#if MagickLibVersion < 0x664
#error ImageMagick Version 6.6.4 or higher is required
#endif

//
// ================================== class cOSDImage ===================================================
//
// load image (EPG or logo) with help of imageMagick
// resize image with imageMagick if size differs from request and target size is provided
// return pointer to cImage with GetImage()
cOSDImage::cOSDImage(cString Filename, int Width, int Height, tColor ClrBorder, int Border, int OBevel)
{
   DSYSLOG2("skinelchiHD: cOSDImage EPG: %s", *imagefilename)
   InitializeMagick(NULL);
   
   imagefilename = Filename;
   width = Width;
   height = Height;
   clrBorder = ClrBorder;
   border = Border;
   oBevel = OBevel;
   image = NULL;
   if (!isempty(imagefilename))
      LoadImage(false);
}

cOSDImage::cOSDImage(cString Filename, int Width, int Height, tColor ClrBorder, int Border)
{
   DSYSLOG2("skinelchiHD: cOSDImage Logo: %s", *imagefilename)
   InitializeMagick(NULL);
   
   imagefilename = Filename;
   width = Width;
   height = Height;
   clrBorder = ClrBorder;
   border = Border;
   oBevel = -1;
   image = NULL;
   if (!isempty(imagefilename))
      LoadImage(true);
}

cOSDImage::~cOSDImage()
{
   DSYSLOG2("skinelchiHD: cOSDImage Desctructor: %s", *imagefilename)
}

bool cOSDImage::LoadImage(bool isLogo)
{   
   DSYSLOG2("skinelchiHD: Loading cOSDImage: %s", *imagefilename)

#ifdef DEBUG_IMAGETIMES
   double tp1, tp2, tp3, tp4;
   tp1 = GetTimeMS();
#endif

   Magick::Color mgkBG = Magick::Color(QuantumRange/255*((clrBorder & 0x00FF0000)>>16),
                                       QuantumRange/255*((clrBorder & 0x0000FF00)>> 8),
                                       QuantumRange/255* (clrBorder & 0x000000FF), 
                                       QuantumRange/255*((clrBorder & 0xFF000000)>>24)); //RGBA
   try {
      Image mgkImage(Magick::Geometry(width-2*border, height-2*border), "None");

      if (isLogo) {
#if MagickLibVersion < 0x700
         mgkImage.type(Magick::TrueColorMatteType);
#else
         mgkImage.type(Magick::TrueColorAlphaType);
#endif
         mgkImage.backgroundColor("None");
      }
      else {
         mgkImage.type(Magick::TrueColorType);
      }

      mgkImage.read(Magick::Geometry(width-2*border, height-2*border), *imagefilename);

#ifdef DEBUG_IMAGETIMES
      tp2 = GetTimeMS();
#endif

      if ( 0 == mgkImage.fileSize()) {
         esyslog("skinelchiHD: ERROR loading cOSDImage '%s'", (const char *)imagefilename);
         return false;
      }
      else 
      {
         int wImg = mgkImage.columns();
         int hImg = mgkImage.rows();

         if (isLogo) { // Logo
            mgkImage.resize(Magick::Geometry(width-2*border, height-2*border));
            mgkImage.extent(Magick::Geometry(width, height), mgkBG, MagickCore::CenterGravity);
         }
         else { // EPG image
            // reisze if required without changing aspect ratio
            if (hImg != height - 2*border) {  //TODO check if height is greater than allowed, then shrink width
               mgkImage.resize(Geometry((width -2*border)*wImg/hImg, height - 2*border, 0, 0));
            }
 
            if (border) {
               if (oBevel) { // EPG image with beveled frame
                  mgkImage.borderColor(mgkBG);
                  mgkImage.frame(border, border, border-oBevel, oBevel);
               }
            }
         }
         wImg = mgkImage.columns();
         hImg = mgkImage.rows();
 
#ifdef DEBUG_IMAGETIMES
         tp3 = GetTimeMS();
#endif
         DSYSLOG("skinelchiHD: cOSDImage: 4 %dx%d  Magick:%dx%d", width, height, (int)mgkImage.columns(), (int)mgkImage.rows());
         
         // convert to cImage
         mgkImage.depth(8);
         mgkImage.getPixels (0, 0, wImg, hImg); // x, y, w, h
         image = new cImage( cSize(wImg, hImg), NULL);
         mgkImage.writePixels(MagickCore::BGRAQuantum, (unsigned char *) image->Data());
         
#ifdef DEBUG_IMAGETIMES
         tp4 = GetTimeMS();
#endif
      }
   }
   
   catch( Exception &error_ ) 
   { 
      esyslog("skinelchiHD: exception reading cOSDImage '%s': %s", (const char *)imagefilename, error_.what());
#ifdef DEBUG_IMAGETIMES
      tp2 = tp3 = tp4 = tp1;
#endif
                  
      return false;
   }

#ifdef DEBUG_IMAGETIMES
   DSYSLOG("skinelchiHD: cOSDImage load: %4.3f ms, resize: %4.3f ms, transfer: %4.3f ms %s", tp2 - tp1, tp3 - tp2, tp4 - tp3, (const char *)imagefilename);
#endif

   return true;
}


void DrawShadedRectangle(cPixmap *Pm, tColor Color, const cRect &Area)
{
   // if Area is NULL draw complete pixmap, else only Area
   cRect shadeArea = (&Area == &cRect::Null) ? Pm->DrawPort() : Area;

   int h = shadeArea.Height();
   int w = shadeArea.Width();
   int x = shadeArea.Left();
   int y = shadeArea.Top();

#ifdef DEBUG_IMAGETIMES   
   double tp1, tp2;
   tp1 = GetTimeMS();
#endif
   
   double shaded = h/2;
   LOCK_PIXMAPS;
   for (int line = 0; line < h; line++) {
      double factor = line <= h/2
                    ? (shaded-line)*(shaded-line)/(shaded*shaded)*0.6        // tint upper part
                    : (line-h+shaded)*(line-h+shaded)/(shaded*shaded)*-0.75; // shade lower part
      Pm->DrawRectangle(cRect(x, y+line, w, 1), RgbShade(Color, factor));
   }

#ifdef DEBUG_IMAGETIMES   
   tp2 = GetTimeMS();
   DSYSLOG("skinelchiHD: DrawShadedRect: %d %d %d-%d %d-%d  %4.3f ms", h, w, x, Pm->DrawPort().X(), y, Pm->DrawPort().Y(), tp2 - tp1);
#endif
}
