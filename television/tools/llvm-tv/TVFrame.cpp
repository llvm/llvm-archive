//===-- TVFrame.cpp - LLVM-TV Main GUI ---------------------------*- C++ -*--=//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "wx/wx.h"
#include "TVFrame.h"

TVFrame::TVFrame(const wxChar *title,
		       int xpos, int ypos,
		       int width, int height)
  : wxFrame ( (wxFrame *) NULL,
	      -1, title, wxPoint(xpos, ypos),
	      wxSize(width, height)
	      ) {

  sampleText = new wxTextCtrl( this,-1,
			       wxString("This is a text control\n\n"
					"The text control supports"
					" basic text editing operations\n" 
					"along with copy, cut, paste, "
					"delete, select all and undo.\n\n" 
					"Right click on the control"
					" to see the pop-up menu.\n" 
					),
			       wxDefaultPosition,
			       wxDefaultSize,
			       wxTE_MULTILINE
			       );  

  fileMenu = new wxMenu;
  fileMenu->Append(BASIC_OPEN,  "&Open file");
  fileMenu->Append(BASIC_ABOUT, "&About");
  fileMenu->AppendSeparator();
  fileMenu->Append(BASIC_EXIT,  "E&xit");
  
  menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  SetMenuBar(menuBar);
}

TVFrame::~TVFrame() {

}
