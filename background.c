   /*
 * background.c: thread for background tasks
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

//#define DEBUG
//#define DEBUG2
#include "common.h"
#include "background.h"
#include "config.h"

#define DELAY (uint64_t)20   // 20ms = 50 updates per second (fps)


///
/// ================================== class cElchiBackground ===================================================
/// calls regular update tasks in background
/// tasks are maintained in a list
/// requires valid cOsd to update OSD, so call SetOSD whenever OSD is created/destroyed 
cElchiBackground::cElchiBackground(void)
:cThread("skinelchiHD-BG")
{
   osd = NULL;
}


cElchiBackground::~cElchiBackground()
{
   Stop();

   int cnt = 0;
   mtxBg.Lock();
   while (bgObjectList.Count() > 0) {
      cvBlock.TimedWait(mtxBg, 2);
      cnt++;
   }
   mtxBg.Unlock(); 
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
   cMutexLock mtxBgLock(&mtxBg);
   bgObjectList.Add(bgObject);
   cvBlock.Broadcast();
}


void cElchiBackground::Del(cBgObject* bgObject)
{
   cMutexLock mtxBgLock(&mtxBg);
   bgObjectList.Del(bgObject, false);
   cvBlock.Broadcast();
}


void cElchiBackground::Action(void)
{
   cTimeMs timer;
   uint64_t delay = -1;
   
   while (Running()) {
      mtxBg.Lock(); // Multicore Lst 3.10
      while (Running() && bgObjectList.Count() == 0) {
         cvBlock.Wait(mtxBg);  // give mtxBg back until condition is met (ObjectList not empty), re-gain lock automatically
      }

      timer.Set(0);
      bool doFlush = false;

      // check each object if update is needed
      for (cBgObject *bgObj = bgObjectList.First(); bgObj; bgObj = bgObjectList.Next(bgObj)) {
         // pixmap locking needs to be done in each Update() function, not globally
         doFlush |= bgObj->Update();
      }
      
      mtxBg.Unlock();  // give mtxBg back

      if (Running() && osd && doFlush) {
         osd->Flush();
      }

      delay = std::max(DELAY - std::min(DELAY, timer.Elapsed()), DELAY/10);
      if (Running()) {
         cwDelay.Wait(delay);
      }

   } // while Running()
}



///
/// ================================== class cScrollingPixmap ===================================================
///
/// create a pixmap and add it to the background thread for scrolling
///
cScrollingPixmap::cScrollingPixmap(cOsd *Osd, const cRect VPort, const cFont *Font, int max_char, tColor ColorFg, tColor ColorBg, bool Centered, int Alignment)
{
   osd = Osd;
   vPort = VPort;
   colorFg = ColorFg;
   colorBg = ColorBg;
   centered = Centered;
   alignment = Alignment;
   cSize const maxSize = osd->MaxPixmapSize();  //TODO  create and scroll multiple pixmaps if one is not sufficient
   maxwidth = std::min(maxSize.Width(), max_char * Font->Width("M")); // assuming M is widest character
   direction = 0;
   active = false;
   text = cString(NULL);
   
   if ((alignment & taBorder) != 0)
   {
      vPort.SetX(vPort.X() + max(Font->Height() / TEXT_ALIGN_BORDER, 1));
      vPort.SetWidth(vPort.Width() - 2* max(Font->Height() / TEXT_ALIGN_BORDER, 1));
   }
   
   pixmap = osd->CreatePixmap(LYR_SCROLL, vPort, cRect(0, 0, maxwidth, Font->Height()));
   pixmap->Clear();
}


cScrollingPixmap::~cScrollingPixmap()
/// caller must not lock pixmaps before calling destructor
{
   if (active)
      ElchiBackground->Del(this);
   osd->DestroyPixmap(pixmap);
}


void cScrollingPixmap::SetText(const char *Text, const cFont *Font)
{  
   //skip on identical or no Text
   if ((NULL == Text) && (NULL ==  *text))
      return;
   if ((NULL != *text) && (NULL != Text) && !strcmp(*text, Text))
      return;

   if (active) {
      ElchiBackground->Del(this);
      active = false;
   }
   text = cString(Text);
   if (Text)
      textWidth = min(maxwidth, Font->Width(*text));
   else
      textWidth = 0;

   if (ElchiConfig.useScrolling && textWidth > vPort.Width())
   {
      xoffset = 0;
      maxXoffset = textWidth - vPort.Width();
      spmTimer.Set(0);
      direction = -1;
      Delay = 5*10;
   }
   else
   {
      xoffset = 0;
      direction = 0; // no scrolling
   }

   if (centered && textWidth <= vPort.Width())
   {
      LOCK_PIXMAPS;
      pixmap->Clear();
      if (isempty(text))
         pixmap->Fill(colorBg);
      else
         pixmap->DrawText(cPoint(0, 0), *text, colorFg, colorBg, Font, maxwidth, Font->Height(), taDefault);
      
      pixmap->SetDrawPortPoint(cPoint((vPort.Width() - textWidth)/2 , 0));
   }
   else
   {
      {  // locking block
         LOCK_PIXMAPS;
         pixmap->Clear();
         if (NULL == (const char *)text)
            pixmap->Fill(colorBg);
         else
            pixmap->DrawText(cPoint(0, 0), *text, colorFg, colorBg, Font, max(vPort.Width(), textWidth), Font->Height(), alignment & ~taBorder);
      
         pixmap->SetDrawPortPoint(cPoint(0, 0));
      }
      ElchiBackground->Add(this);
      active = true;
   }
}


bool cScrollingPixmap::Update()
{  // mtxBg is locked when Update() is called, Update() can lock Pixmaps
   bool changed = false;
   int delay = 5;
   uint64_t intervall = 20;
   
   // check if update is needed
   if (direction) {

      uint64_t elapsed = spmTimer.Elapsed() / intervall;
      if (elapsed) {
         if (Delay > 0) {
            int x = elapsed / Delay;
            Delay -= elapsed;
            if (Delay <= 0) {
               xoffset -= direction * x;
               changed = true;
            }
            else {
               spmTimer.Set();
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
            spmTimer.Set();
         }
      }

      if (changed) {
         pixmap->SetDrawPortPoint(cPoint(-xoffset, 0));
      }
   }
   return changed;
}


  
///
/// ================================== class cEpgImage ===================================================
///
/// load and buffer EPG images
/// provide pointer to EPG image on request
/// load EPG image in separate thread
/// Instance exists from SetEvent until cMenu is closed
///
cEpgImage::cEpgImage(cPixmap *Pixmap, int Width, int Height, int FrameSize)
:cThread("skinelchiHD-cEpgImage")
{
   DSYSLOG("skinelchiHD: cEpgImage");
   pixmap = Pixmap;
   w = Width;
   h = Height;
   frameSize = FrameSize;
   active = false;
   eventID = 0;
   channelID = NULL;
   recordingPath = NULL;
   maxImage = 0;
   currentImage = -1;

   Start();
}


cEpgImage::~cEpgImage()
{
   if (active) {
      ElchiBackground->Del(this);
   }
   Stop();
   Clear();
}


void cEpgImage::Stop()
{
   if (Running()) {
      Cancel(-1);
      condWait.Signal();
      Cancel(2);  // avoid uninitialized mutex in Clear()
   }
}


/// delete all cached images
void cEpgImage::Clear()
{
   DSYSLOG("skinelchiHD: cEpgImage::Clear() (%d)", maxImage);
   cMutexLock mtxImagesLock(&mtxImages);
   if (maxImage > 0)
      for (int i = 0; i<maxImage; i++)
         delete imgEPG[i];
   maxImage = 0;
   currentImage = -1;
}


bool cEpgImage::PutEventID(const char *ChannelID, tEventID EventID)
{
   DSYSLOG("skinelchiHD: cEpgImage::PutEventID(%s %d)", (const char *)ChannelID, EventID);
   int rc = -1;
   cString filename;
   struct stat statbuf;
   
   mtxEventID.Lock();
   channelID = cString(ChannelID);
   eventID = EventID;
   recordingPath = NULL;
   mtxEventID.Unlock();

   if (eventID == 0) { // on empty eventID
      if (active) {
         ElchiBackground->Del(this);
         active = false;
      }
   }
   else
   {
      filename = cString::sprintf("%s/%s_%d.jpg", ElchiConfig.GetEpgImageDir(), *channelID, eventID);
      rc = stat(filename, &statbuf);
      if (rc != 0)
      {
         filename = cString::sprintf("%s/%s_%d_0.jpg", ElchiConfig.GetEpgImageDir(), *channelID, eventID);
         rc = stat(filename, &statbuf);
         /* eventID without channelID gives often wrong pictures because of same eventID on different channels
          * therefore only active when no picture with channelID found and event-only is enabled in settings */
         if (rc != 0 && ElchiConfig.EpgImageEventIdOnly == 1) {
            filename = cString::sprintf("%s/%d.jpg", ElchiConfig.GetEpgImageDir(), eventID);
            rc = stat(filename, &statbuf);
            if (rc != 0) {
               filename = cString::sprintf("%s/%d_0.jpg", ElchiConfig.GetEpgImageDir(), eventID);
               rc = stat(filename, &statbuf);
            }
            if (rc == 0) channelID = NULL;
         }
      }
      
      // start Thread only if image(s) exist
      if (rc == 0) {
         condWait.Signal();
         ElchiBackground->Add(this);
         active = true;
      }
   }
         
   return rc == 0;
}


bool cEpgImage::PutRecording(const char *RecPath)
{
   //DSYSLOG("skinelchiHD: cEpgImage::PutRecording (%s ?== %s)", (const char *)RecPath, *recordingPath)
   bool imagesAvailable = false;
   struct dirent *e;
   char *dot;

   if (active) {
      ElchiBackground->Del(this);
      active = false;
   }

   // check if at least one image is available
   cReadDir dir(RecPath);
   while ((e = dir.Next()) != NULL) {
      if ((dot = strrchr(e->d_name, '.')) != NULL && (strlen(dot) == 4) ) {
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
      ElchiBackground->Add(this);
      active = true;
   }
   
   return imagesAvailable;
}


/// Update the OSD, e.g. draw picture
bool cEpgImage::Update()  
{
   bool flushRequired = false;
   cMutexLock mtxImagesLock(&mtxImages);

   DSYSLOG2("cEpgImage::Update: maxImage=%d", maxImage);
   if (maxImage > 0)
   {
      if (currentImage == -1 || 
         (epgimageTimer.TimedOut() && (maxImage != 1 || currentImage != 0)))  // avoid continous drawing if only one image exists
      {
         currentImage++;
         
         if (currentImage >= maxImage)
            currentImage = 0;
         
         {  // locking block
            LOCK_PIXMAPS;
            pixmap->DrawImage(cPoint(frameSize, frameSize), *imgEPG[currentImage]->GetImage());
            pixmap->SetLayer(LYR_TEXT);
         }
         
         epgimageTimer.Set(ElchiConfig.EpgImageDisplayTime * 1000);
         flushRequired = true;
      }
   }
   return flushRequired;
}

/// thread for reading all images
void cEpgImage::Action(void)
{
   condWait.Wait();  // wait for start signal

   while (Running()) {
      DSYSLOG2("skinelchiHD: cEpgImage::Action start %d", eventID)

      mtxEventID.Lock();  // enters at beginning or after wake up from PutEventID()
      tEventID currentEventID = eventID;
      cString currentChannelID = cString(channelID);
      cString currentRecordingPath = cString(recordingPath);
      mtxEventID.Unlock();
      
      Clear();

      if (currentEventID > 0) {  // new task exists
         cString filename;
         struct stat statbuf;
         int rc = -1;
         cStringList recImages;
         
         // init image variables
         if (isempty(currentRecordingPath))
         {  // EPG image
            if (*channelID) {
               filename = cString::sprintf("%s/%s_%d.jpg", ElchiConfig.GetEpgImageDir(), *currentChannelID, currentEventID);
               rc = stat(filename, &statbuf);
               if ( rc != 0)
                  filename = cString::sprintf("%s/%s_%d_0.jpg", ElchiConfig.GetEpgImageDir(), *currentChannelID, currentEventID);
            }
            else { /* Caution: eventID without channel gives often wrong pictures */
               filename = cString::sprintf("%s/%d.jpg", ElchiConfig.GetEpgImageDir(), currentEventID);
               rc = stat(filename, &statbuf);
               if ( rc != 0)
                  filename = cString::sprintf("%s/%d_0.jpg", ElchiConfig.GetEpgImageDir(), currentEventID);
            }
            rc = stat(filename, &statbuf);
         }
         else
         {  // recording image
            cReadDir dir(currentRecordingPath);
            struct dirent *e;
            char *dot;
            rc = -1;
            
            while ((e = dir.Next()) != NULL) {
               if ((dot = strrchr(e->d_name, '.')) != NULL && (strlen(dot) == 4) ) {
                  if(strcasecmp(dot, ".jpg") == 0)
                     recImages.Append(strdup(cString::sprintf("%s/%s", *recordingPath, e->d_name)));
               }
            }
            if (recImages.Size() > 0) {
               recImages.Sort();
               filename = recImages[0];
               rc = 0;
            }
         }

         // loop over all images
         while (rc == 0 && maxImage < MAXEPGIMAGES && Running()) { // start conversion only if at least one image exists
            DSYSLOG2("skinelchiHD: cEpgImage::Action %d %s %d", rc, *filename, maxImage)

            imgEPG[maxImage] = new cOSDImage(filename, w, h);
            
            if (imgEPG[maxImage] != NULL && imgEPG[maxImage]->GetImage())
            {
               cMutexLock mtxImagesLock(&mtxImages);
               maxImage++;
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
}
