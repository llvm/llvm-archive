//===-- TransformVisualizer.cpp - GUI --------*- C++ -*--=//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "wx/wx.h"
#include "TVApplication.h"
#include "TVFrame.h"

IMPLEMENT_APP(TVApplication)

bool TVApplication::OnInit() {
  
  TVFrame *frame = new TVFrame("llvm-tv", 50, 50, 450, 300);
  
  frame->Show(TRUE);
  SetTopWindow(frame);
  return TRUE;	
}
