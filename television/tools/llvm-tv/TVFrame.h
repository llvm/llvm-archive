//===-- TVFrame.h - Main llvm-tv GUI -----------------------------*- C++ -*--=//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#ifndef TVFRAME_H
#define TVFRAME_H

enum
{ BASIC_EXIT    =   1,
  BASIC_OPEN	= 100,
  BASIC_ABOUT	= 200
};

class TVFrame : public wxFrame {
public:
  TVFrame( const wxChar *title, 
	   int xpos, int ypos, 
	   int width, int height);
  ~TVFrame();

  wxTextCtrl *sampleText;
  wxMenuBar  *menuBar;
  wxMenu     *fileMenu;

};


#endif
