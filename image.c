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

//#define DEBUG
//#define DEBUG_IMAGETIMES

#include "image.h"
#include "common.h"
#include <sys/time.h> 

using namespace std; //required by exception handling
using namespace Magick;

#ifdef GRAPHICSMAGICK
#if !defined(MagickLibVersion)
#error GraphicsMagick is required
#endif
#else
#if !defined(MagickLibVersion) || MagickLibVersion < 0x700
#error ImageMagick Version 7.0 or higher is required
#endif
#endif

//
// ================================== class cOSDImage ===================================================
//
// load image (EPG or logo) with help of imageMagick
// resize image with imageMagick if size differs from request and target size is provided
// return pointer to cImage with GetImage()
cOSDImage::cOSDImage(cString Filename, int Width, int Height)
{
   DSYSLOG2("skinelchiHD: cOSDImage EPG: %s", *imagefilename)
   InitializeMagick(NULL);

   imagefilename = Filename;
   width = widthImage = Width;
   height = heightImage = Height;
   border = 0;
   image = NULL;
   if (!isempty(imagefilename))
      LoadImage(false);
}

cOSDImage::cOSDImage(cString Filename, int Width, int Height, int Border)
{
   DSYSLOG2("skinelchiHD: cOSDImage Logo: %s", *imagefilename)
   InitializeMagick(NULL);

   imagefilename = Filename;
   width = widthImage = Width;
   height = heightImage = Height;
   border = Border;
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

   try {
      Image mgkImage(Geometry(width-2*border, height-2*border), "None");

      if (isLogo) {
#ifdef GRAPHICSMAGICK
         mgkImage.type(TrueColorMatteType);
#else
         mgkImage.type(TrueColorAlphaType);
#endif
         mgkImage.backgroundColor("None");
      }
      else {
         mgkImage.type(TrueColorType);
      }

      mgkImage.read(Geometry(width-2*border, height-2*border), *imagefilename);

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
            mgkImage.resize(Geometry(width-2*border, height-2*border));
            mgkImage.extent(Geometry(width, height), "None", CenterGravity);
         }
         else { // EPG image
            if (hImg != height - 2*border) { // reisze if required without changing aspect ratio
               mgkImage.resize(Geometry((width -2*border)*wImg/hImg, height - 2*border, 0, 0));
            }
            if ((int)mgkImage.columns() > width - 2*border) { // cut left/right border if it does not fit
               mgkImage.shave(Geometry(((int)mgkImage.columns() - (width - 2*border))/2, 0, 0, 0));
            }
         }
         widthImage = mgkImage.columns();
         heightImage = mgkImage.rows();

#ifdef DEBUG_IMAGETIMES
         tp3 = GetTimeMS();
#endif
         // convert to cImage
         mgkImage.depth(8);
         mgkImage.getPixels (0, 0, widthImage, heightImage); // x, y, w, h
         image = new cImage( cSize(widthImage, heightImage), NULL);

#ifdef GRAPHICSMAGICK
         mgkImage.writePixels(RGBAQuantum, (unsigned char *) image->Data());
#else
         mgkImage.writePixels(MagickCore::BGRAQuantum, (unsigned char *) image->Data());
#endif

#ifdef DEBUG_IMAGETIMES
         tp4 = GetTimeMS();
#endif

#ifdef GRAPHICSMAGICK
         union uColor *cvrt, buffer;

         cvrt = (union uColor *)image->Data();
         for(int i=0; i<widthImage*heightImage; i++, cvrt++)
         {
            buffer.clr = cvrt->clr;
            cvrt->c.B = buffer.c.R;
            cvrt->c.R = buffer.c.B;
         }
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
   isyslog("skinelchiHD: cOSDImage load: %4.3f ms, resize: %4.3f ms, transfer: %4.3f ms %s", tp2 - tp1, tp3 - tp2, tp4 - tp3, (const char *)imagefilename);
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
   isyslog("skinelchiHD: DrawShadedRect: %d %d %d-%d %d-%d  %4.3f ms", h, w, x, Pm->DrawPort().X(), y, Pm->DrawPort().Y(), tp2 - tp1);
#endif
}
