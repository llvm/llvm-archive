/*
 * LLBrowseApp.cpp
 */

#include "llbrowse/BrowserApp.h"
#include "llbrowse/BrowserFrame.h"

#include "llvm/Support/ManagedStatic.h"
#if HAVE_LLVM_SUPPORT_SIGNALS_H
#include "llvm/Support/Signals.h"
#endif

#include "wx/config.h"

using namespace llvm;

IMPLEMENT_APP(BrowserApp)

static const wxString CONFIG_APP_NAME = wxString::FromAscii("llbrowse");

static const wxString CGROUP_WINDOW = wxString::FromAscii("/Window");
static const wxString CKEY_LEFT = wxString::FromAscii("left");
static const wxString CKEY_TOP = wxString::FromAscii("top");
static const wxString CKEY_WIDTH = wxString::FromAscii("width");
static const wxString CKEY_HEIGHT = wxString::FromAscii("height");

static const wxString CGROUP_PATHS = wxString::FromAscii("/Paths");
static const wxString CKEY_CURRENTDIR = wxString::FromAscii("currentdir");

bool BrowserApp::OnInit() {
  #if HAVE_LLVM_SUPPORT_SIGNALS_H
    sys::PrintStackTraceOnErrorSignal();
  #endif

  wxImage::AddHandler(new wxPNGHandler);

  // Read saved settings
  wxPoint initialPos = wxDefaultPosition;
  wxSize initialSize = wxDefaultSize;
  wxString initialDir;
  wxConfig* config = new wxConfig(CONFIG_APP_NAME);
  if (config != NULL) {
    config->SetPath(CGROUP_WINDOW);
    config->Read(CKEY_LEFT, &initialPos.x);
    config->Read(CKEY_TOP, &initialPos.y);
    config->Read(CKEY_WIDTH, &initialSize.x);
    config->Read(CKEY_HEIGHT, &initialSize.y);

    config->SetPath(CGROUP_PATHS);
    config->Read(CKEY_CURRENTDIR, &initialDir);
  }

  frame_ = new BrowserFrame(_("LLVM Module Browser"), initialPos, initialSize);
  if (!initialDir.IsEmpty()) {
    frame_->SetCurrentDir(initialDir);
  }

  // TODO: Parse command-line flags and begin loading a module if one is
  // specified on the command line.

  SetTopWindow(frame_);
  return true;
}

int BrowserApp::OnExit() {
  llvm_shutdown();
  return wxApp::OnExit();
}

void BrowserApp::SavePrefs() {
  wxConfig* config = new wxConfig(CONFIG_APP_NAME);
  if (config != NULL) {
    if (frame_ != NULL) {
      wxPoint pos = frame_->GetPosition();
      wxSize size = frame_->GetSize();
      wxString currentDir = frame_->GetCurrentDir();
      config->SetPath(CGROUP_WINDOW);
      config->Write(CKEY_LEFT, pos.x);
      config->Write(CKEY_TOP, pos.y);
      config->Write(CKEY_WIDTH, size.x);
      config->Write(CKEY_HEIGHT, size.y);

      config->SetPath(CGROUP_PATHS);
      if (!currentDir.IsEmpty()) {
        config->Write(CKEY_CURRENTDIR, currentDir);
      } else {
        config->DeleteEntry(CKEY_CURRENTDIR, true);
      }
    }

    delete config;
  }
}
