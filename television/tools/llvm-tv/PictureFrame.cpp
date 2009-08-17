#include "PictureFrame.h"
#include "TVApplication.h"

// PictureFrame implementation

void PictureFrame::OnClose(wxCloseEvent &event) {
  myApp->GoodbyeFrom (this);
  Destroy();
}

BEGIN_EVENT_TABLE(PictureFrame, wxFrame)
  EVT_CLOSE(PictureFrame::OnClose)
END_EVENT_TABLE()
