//===-- TransformVisualizer.cpp - GUI --------*- C++ -*--=//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "wx/wx.h"
#include "TVApplication.h"
#include "TVFrame.h"
#include "llvm-tv/Config.h"
#include <fstream>
#include <sys/types.h>
#include <unistd.h>

IMPLEMENT_APP(TVApplication)

bool TVApplication::OnInit() {
  // Save my pid into a file  
  std::ofstream pidFile(llvmtvPID.c_str());
  if (pidFile.good() && pidFile.is_open()) {
    pidFile << getpid();
    pidFile.close();
  }

  TVFrame *frame = new TVFrame("llvm-tv", 50, 50, 450, 300);
  
  frame->Show(TRUE);
  SetTopWindow(frame);
  return TRUE;	
}
