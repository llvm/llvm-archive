/*
 * LLBrowseApp.h
 */

#ifndef LLBROWSE_BROWSERAPP_H
#define LLBROWSE_BROWSERAPP_H

#ifndef _WX_WX_H_
#include "wx/wx.h"
#endif

class BrowserFrame;

class BrowserApp : public wxApp {
public:
  bool OnInit();
  int OnExit();

  void SavePrefs();

private:
  BrowserFrame* frame_;
};

DECLARE_APP(BrowserApp)

#endif // LLBROWSE_BROWSERAPP_H
