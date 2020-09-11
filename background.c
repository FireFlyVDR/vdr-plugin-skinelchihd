   /*
 * background.c: thread for background tasks
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include "common.h"
#include "background.h"
#include "config.h"

#define DELAY 20   // 20ms = 50 updates per second

//
// ================================== class cElchiBackground ===================================================
// calls regular update tasks in background
// tasks are maintained in a list
// requires valid cOsd to update OSD, so call SetOSD whenever OSD is created/destroyed 

cElchiBackground::cElchiBackground(void)
:cThread("skinelchiHD-BG")
{
   osd = NULL;
   bgObjectList.Clear();
}


cElchiBackground::~cElchiBackground()
{
   Stop();

   cMutexLock(mtxBg);
   ///bgObjectList.Clear();
   for (cBgObject *bgObj = bgObjectList.First(); bgObj; bgObj = bgObjectList.Next(bgObj)) {
      Del(bgObj, false);
   }
}


void cElchiBackground::Start()
{
   cThread::Start();
}


void cElchiBackground::Stop()
{
   if (Running()) {
      Cancel(-1);
      cwDelay.Signal();
      cvBlock.Broadcast();
      Cancel(10);
   }
}


void cElchiBackground::Add(cBgObject* bgObject)
{
   cMutexLock(mtxBg);
   bgObjectList.Add(bgObject);
   cvBlock.Broadcast();
}


void cElchiBackground::Del(cBgObject* bgObject, bool delObject)
{
   mtxBg.Lock();
   bgObjectList.Del(bgObject, delObject);
   cvBlock.Broadcast();
   mtxBg.Unlock();
}


void cElchiBackground::Action(void)
{
   cTimeMs timer;

   while (Running()) {
      mtxBg.Lock(); // Multicore Lst 3.10
      while (Running() && bgObjectList.Count() == 0) {
         cvBlock.Wait(mtxBg);  // give mtxBg back until condition is met (ObjectList not empty), re-gain lock automatically
      }

      timer.Set(0);
      bool doFlush = false;

      // check each object if update is needed
      for (cBgObject *bgObj = bgObjectList.First(); bgObj; bgObj = bgObjectList.Next(bgObj)) {
         // LOCK_PIXMAPS; // locking needs to be done in each Update() function, not globally
         doFlush |= bgObj->Update();
      }
      
      mtxBg.Unlock();  // give mtxBg back

      if (Running() && osd && doFlush) {
         osd->Flush();
      }

      uint64_t delay = DELAY - min((uint64_t)DELAY, timer.Elapsed());
      if (Running() && delay > 0) {
         cwDelay.Wait(delay);
      }

   } // while Running()
}



//
// ================================== class cScrollingPixmap ===================================================
//
// create a pixmap and add it to the background thread for scrolling
//
cScrollingPixmap::cScrollingPixmap(cOsd *Osd, const cRect VPort, const cFont *Font, int max_char, tColor ColorFg, tColor ColorBg, bool Centered, int Alignment)
{
   osd = Osd;
   vPort = VPort;
   colorFg = ColorFg;
   colorBg = ColorBg;
   centered = Centered;
   alignment = Alignment;
   cSize const maxSize = osd->MaxPixmapSize();  //TODO  create and scroll multiples pixmaps if one is not sufficient
   maxwidth = std::min(maxSize.Width(), max_char * Font->Width("M")); // assuming M is widest character
   direction = 0;
   text = cString(NULL);
   
   if ((alignment & taBorder) != 0)
   {
      vPort.SetX(vPort.X() + max(Font->Height() / TEXT_ALIGN_BORDER, 1));
      vPort.SetWidth(vPort.Width() - 2* max(Font->Height() / TEXT_ALIGN_BORDER, 1));
   }
   
   pixmap = osd->CreatePixmap(LYR_SCROLL, vPort, cRect(0, 0, maxwidth, Font->Height()));
   pixmap->Clear();

   ElchiBackground->Add(this);
}


cScrollingPixmap::~cScrollingPixmap()
/// caller must not lock pixmaps before calling destructor
{
   ElchiBackground->Del(this, false);
   osd->DestroyPixmap(pixmap);
}


void cScrollingPixmap::SetText(const char * Text, const cFont * Font)
{  
   //skip if Text is identical
   if ((NULL == Text) && (NULL ==  *text))
      return;
   if ((NULL != *text) && (NULL != Text) && !strcmp(*text, Text))
      return;
   
   text = cString(Text);
   if (Text)
      textWidth = min(maxwidth, Font->Width(*text));
   else
      textWidth = 0;

   if (ElchiConfig.useScrolling && textWidth > vPort.Width())
   {
      xoffset = 0;
      maxXoffset = textWidth - vPort.Width();
      timer.Set(0);
      direction = -1;
      Delay = 5*10;
   }
   else
      direction = 0; // no scrolling

   if (centered && textWidth <= vPort.Width())
   {
      LOCK_PIXMAPS;
      pixmap->Clear();
      if (NULL == (const char *)text)
         pixmap->Fill(colorBg);
      else
         pixmap->DrawText(cPoint(0, 0), *text, colorFg, colorBg, Font, maxwidth, Font->Height(), taDefault);
      
      pixmap->SetDrawPortPoint(cPoint((vPort.Width() - textWidth)/2 , 0));
   }
   else
   {
      LOCK_PIXMAPS;
      pixmap->Clear();
      if (NULL == (const char *)text)
         pixmap->Fill(colorBg);
      else
         pixmap->DrawText(cPoint(0, 0), *text, colorFg, colorBg, Font, max(vPort.Width(), textWidth), Font->Height(), alignment & ~taBorder);
      
      pixmap->SetDrawPortPoint(cPoint(0, 0));
   }
}


void cScrollingPixmap::SetColor(tColor ColorFg, tColor ColorBg)
{
   colorFg = ColorFg;
   colorBg = ColorBg;
}


void cScrollingPixmap::SetLayer(int Layer)
{
   pixmap->SetLayer(Layer);
}


void cScrollingPixmap::SeAlpha(int Alpha)
{
   pixmap->SetAlpha(Alpha);
}


void cScrollingPixmap::SetViewPort(const cRect &Rect)
{
   pixmap->SetViewPort(Rect);
}


bool cScrollingPixmap::Update()
{
   bool changed = false;
   int delay = 5;
   uint64_t intervall = 20;
   
   // check if update is needed
   if (direction) {

      uint64_t elapsed = timer.Elapsed() / intervall;
      if (elapsed) {
         if (Delay > 0) {
            int x = elapsed / Delay;
            Delay -= elapsed;
            if (Delay <= 0) {
               xoffset -= direction * x;
               changed = true;
            }
            else {
               timer.Set();
               changed = false;
            }
         }
         else {
            xoffset -= direction * elapsed;
            changed = true;
         }

         if (changed) {
            if (xoffset < 0) {
               xoffset = 0;
               direction = -1;
            }
            else {
               if (xoffset > maxXoffset) {
                  xoffset = maxXoffset;
                  direction = 1;
               }
            }
            Delay = delay - ((maxXoffset - xoffset > xoffset) ? xoffset : maxXoffset - xoffset);
            timer.Set();
         }
      }

      if (changed) {    // SetDrawPortPoint sets LOCK_PIXMAP
         pixmap->SetDrawPortPoint(cPoint(-xoffset, 0));
      }
   }
   return changed;
}


  
//
// ================================== class cEpgImage ===================================================
//
// load and buffer EPG images
// provide pointer to EPG image on request
// load EPG image in separate thread
//
cEpgImage::cEpgImage(cPixmap *Pixmap, int Width, int Height, int FrameSize)
:cThread("skinelchiHD-cEpgImage")
{
   pixmap = Pixmap;
   w = Width;
   h = Height;
   frameSize = FrameSize;
   eventID = 0;
   channelID = NULL;
   recordingPath = NULL;
   maxImage = 0;
   currentImage = -1;

   Start();
   ElchiBackground->Add(this);
   // Instanz existiert von SetEvent bis cMenu geschlossen wird
}


cEpgImage::~cEpgImage()
{
   ElchiBackground->Del(this, false);
   Stop();
}


void cEpgImage::Stop()
{
   if (Running()) {
      Cancel(-1);
      condWait.Signal();
   }
}


bool cEpgImage::PutEventID(const char *ChannelID, tEventID EventID)
{
   DSYSLOG("skinelchiHD: cEpgImage::PutEventID(%s %d)", (const char *)ChannelID, EventID)
   int rc = -1;
   cString filename;
   struct stat statbuf;
   
   mtxEventID.Lock();
   channelID = cString(ChannelID);
   eventID = EventID;
   recordingPath = NULL;
   mtxEventID.Unlock();

   if (0 == eventID) // skip on empty eventID
      Clear();
   else
   {
      filename = cString::sprintf("%s/%s_%d.jpg", ElchiConfig.GetEpgImageDir(), *channelID, eventID);
      rc = stat(filename, &statbuf);
      /* eventID without channel gives often wrong pictures
      if (rc != 0) {
         filename = cString::sprintf("%s/%d.jpg", ElchiConfig.GetEpgImageDir(), eventID);
         rc = stat(filename, &statbuf);
         if (rc == 0) channelID = NULL;
      }
      */
      
      // start Thread only if image(s) exist
      if (rc == 0)
         condWait.Signal();
   }
         
   return rc == 0;
}


bool cEpgImage::PutRecording(const char *RecPath)
{
   DSYSLOG("skinelchiHD: cEpgImage::PutRecording (%s)", (const char *)RecPath)
   bool imagesAvailable = false;
   struct dirent *e;
   char *dot;
         
   // check if at least one image is available
   cReadDir dir(RecPath);
   while ((e = dir.Next()) != NULL) {
      if ((dot = strrchr(e->d_name, '.')) != NULL ) {
         if(strcasecmp(dot, ".jpg") == 0) {
            imagesAvailable = true;
            break;
         }
      }
   }
   
   if (imagesAvailable)
   {
      mtxEventID.Lock();
      channelID = NULL;
      eventID = 65535;
      recordingPath = cString(RecPath);
      mtxEventID.Unlock();
      condWait.Signal();  // start Thread
   }
   
   return imagesAvailable;
}


// Update the OSD, e.g. draw picture
bool cEpgImage::Update()  
{
   bool flushRequired = false;
   mtxImages.Lock();

   if (maxImage > 0)
   {
      if (currentImage == -1 || 
         (epgimageTimer.TimedOut() && (maxImage != 1 || currentImage != 0)))  // avoid continous drawing if only one image exists
      {
         currentImage++;
         
         if (currentImage >= maxImage)
            currentImage = 0;
         
         {
            LOCK_PIXMAPS;
            pixmap->Clear();
            pixmap->DrawImage(cPoint(pixmap->DrawPort().Width() - imgEPG[currentImage]->GetImage()->Width(), 0), *imgEPG[currentImage]->GetImage());
            pixmap->SetLayer(LYR_TEXT);
         }
         
         epgimageTimer.Set(ElchiConfig.EpgImageDisplayTime * 1000);
         flushRequired = true;
      }
   }
   mtxImages.Unlock();
   return flushRequired;
}


// delete all cached images
void cEpgImage::Clear()
{
   mtxImages.Lock();
   for (int i = 0; i<maxImage; i++)
      delete imgEPG[i];
   maxImage = 0;
   currentImage = -1;
   mtxImages.Unlock();
}


// thread for reading all images
void cEpgImage::Action(void) {
   condWait.Wait();  // wait for start signal

   while (Running()) {
      DSYSLOG2("skinelchiHD: cEpgImage::Action start %d", eventID)

      mtxEventID.Lock();  // enters at beginning or after wake up from  PutEventID()
      tEventID currentEventID = eventID;
      cString currentChannelID = cString(channelID);
      cString currentRecordingPath = cString(recordingPath);
      mtxEventID.Unlock();
      
      Clear();

      if (currentEventID > 0) {  // new tas exists
         cString filename;
         struct stat statbuf;
         int rc = -1;
         cStringList recImages;
         
         // init image variables
         if (isempty(currentRecordingPath))
         {  // EPG image
            filename = cString::sprintf("%s/%s_%d.jpg", ElchiConfig.GetEpgImageDir(), *currentChannelID, currentEventID);
            rc = stat(filename, &statbuf);
            /* eventID without channel gives often wrong pictures
            if (rc != 0) {
               filename = cString::sprintf("%s/%d.jpg", ElchiConfig.GetEpgImageDir(), currentEventID);
               rc = stat(filename, &statbuf);
               if (rc == 0) channelID = NULL;
            }
            */
         }
         else
         {  // recording image
            cReadDir dir(currentRecordingPath);
            struct dirent *e;
            char *dot;
            rc = -1;
            
            while ((e = dir.Next()) != NULL) {
               if ((dot = strrchr(e->d_name, '.')) != NULL ) {
                  if(strcasecmp(dot, ".jpg") == 0) {
                     recImages.Append(strdup(cString::sprintf("%s/%s", *recordingPath, e->d_name)));
                  }
                  if (recImages.Size() > 0) {
                     rc = 0;
                     filename = recImages[0];
                  }
               }
            }
         }

         // loop over all images
         while (rc == 0 && maxImage < MAXEPGIMAGES) { // start thread (conversion) only if image available
            DSYSLOG2("skinelchiHD: cEpgImage::Action %d %s %d", rc, *filename, maxImage)

            imgEPG[maxImage] = new cOSDImage(filename, w, h, Theme.Color(clrChannelDateBg), frameSize, frameSize*0.6);
            
            if (imgEPG[maxImage] != NULL && imgEPG[maxImage]->GetImage())
            {
               mtxImages.Lock();
               maxImage++;
               mtxImages.Unlock();
            }

            // init variables for next image
            if (isempty(currentRecordingPath))
            {  // EPG image
               if (*channelID)
                  filename = cString::sprintf("%s/%s_%d_%d.jpg", ElchiConfig.GetEpgImageDir(), *currentChannelID, currentEventID, maxImage);
               else
                  filename = cString::sprintf("%s/%d_%d.jpg", ElchiConfig.GetEpgImageDir(), currentEventID, maxImage);
               rc = stat(filename, &statbuf);
            }
            else
            {  // recording image
               rc = -1;
               if (maxImage < recImages.Size()) {
                  filename = recImages[maxImage];
                  rc = 0;
               }
            }
         }
      }
      condWait.Wait();
   }

   // delete images
   Clear();
}
