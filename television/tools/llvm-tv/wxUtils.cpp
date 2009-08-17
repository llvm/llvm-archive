
#include "wxUtils.h"
#include <wx/string.h>
#include <string>

wxString wxS(const char* str) {
  return wxString(str, wxConvUTF8);
}

wxString wxS(const std::string &str) {
  return wxString(str.c_str(), wxConvUTF8);
}
