/*
 * vdrstatus.h: Keeping track of several VDR status settings
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_VDRSTATUS_H_
#define __ELCHIHD_VDRSTATUS_H_

#include <vdr/status.h>

class cRecordingEntry : public cListObject
{
private:
   const cDevice *device;
   cString name;
   cString filename;
public:
   cRecordingEntry(const cDevice *Device, const char *Name, const char *FileName);
   ~cRecordingEntry();
   virtual int Compare(const cListObject & listObj) const {
      cRecordingEntry *entry = (cRecordingEntry *) & listObj;
      return strcmp (entry->filename, filename);
   }
   const char *GetFilename() { return *filename; }
   const char *GetName() { return *name; }
   const cDevice *GetDevice() { return device; }
};

typedef enum {
   ar_unknown,
   ar4_3,
   ar16_9,
   arHD,
   arUHD
} eAspectRatio;

struct cVideoInfo
{
   int width;       // in pixels
   int height;      // in pixels
   eAspectRatio aspectratio; // aspect ratio
};


class cSkinElchiStatus : public cStatus
{
private:
   const cDevice *ChannelDevice[MAXDEVICES];
   int ChannelNumber[MAXDEVICES];
   cList<cRecordingEntry> recordinglist;
   int recordingChange;
   char *audioTrack;
   const char *audioChannel;
   const char * const *tracks;
   //int audioTrackIndex;
   //int audioChange;
   int volume;
   int volumeChange;
protected:
   //virtual void ChannelChange(const cChannel *Channel);
   //virtual void TimerChange(const cTimer *Timer, eTimerChange Change);
   virtual void ChannelSwitch(const cDevice *Device, int ChannelNumber, bool LiveView);
   virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On);
   //virtual void Replaying(const cControl *Control, const char *Name, const char *FileName, bool On);
   //virtual void MarksModified(const cMarks *Marks);
   virtual void SetVolume(int Volume, bool Absolute);
   //virtual void SetAudioTrack(int Index, const char * const *Tracks);
   //virtual void SetAudioChannel(int AudioChannel);
   //virtual void SetSubtitleTrack(int Index, const char * const *Tracks);
   //virtual void OsdClear();
   //virtual void OsdTitle(const char *Title);
   //virtual void OsdStatusMessage(const char *Message);
   //virtual void OsdHelpKeys(const char *Red, const char *Green, const char *Yellow, const char *Blue);
   //virtual void OsdItem(const char *Text, int Index);
   //virtual void OsdCurrentItem(const char *Text);
   //virtual void OsdTextItem(const char *Text, bool Scroll);
   //virtual void OsdChannel(const char *Text);
   //virtual void OsdProgramme(time_t PresentTime, const char *PresentTitle, const char *PresentSubtitle, time_t FollowingTime, const char *FollowingTitle, const char *FollowingSubtitle);

public:
   cSkinElchiStatus();
   ~cSkinElchiStatus();
   int GetRecordingChange(int *numRecordings) { if (numRecordings) *numRecordings = recordinglist.Count(); return recordingChange; };   // used in channel
   cString GetRecordingsString(cString prefix);

   //int GetVolumeChange(void) { return volumeChange; };        //used in channel, menu, replay
   int GetVolumeChange(int *newVolume) { if (newVolume) *newVolume = volume; return volumeChange; };        //used in channel, menu, replay
   //int GetVolume(void) { return volume; };                    //used in channel, menu, replay

   //const char *GetAudioTrack(void) { return audioTrack; };     // unused
   //int GetAudioTrackIndex(void) { return audioTrackIndex; };   //unused
   
   //const char *GetAudioChannel(void) { return audioChannel; }; // unused, but channel uses directly device query
   //int GetAudioChange(void) { return audioChange; };

   void GetVideoInfo(cVideoInfo *videoinfo);                 //used in channel, replay
};

#endif //__ELCHIHD_VDRSTATUS_H_
