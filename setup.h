/*
 * setup.h: Setup and configuration file handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_SETUP_H
#define __ELCHIHD_SETUP_H

#include <vdr/plugin.h>
#include "config.h"


class cSkinElchiHDSetup : public cMenuSetupPage
{
private:
   cSkinElchiHDConfig tmpElchiConfig;
   void Setup(void);

protected:
   virtual eOSState ProcessKey(eKeys Key);
   virtual void Store(void);

public:
   cSkinElchiHDSetup(void);
   virtual ~cSkinElchiHDSetup();
};

#endif //__ELCHIHD_SETUP_H
