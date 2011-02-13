#include "llbrowse/Resources.h"

#include "wx/mstream.h"
#include "wx/imaglist.h"

#include "resources/icons/alias.png.h"
#include "resources/icons/collapse.png.h"
#include "resources/icons/constant.png.h"
#include "resources/icons/debug.png.h"
#include "resources/icons/expand.png.h"
#include "resources/icons/folder.png.h"
#include "resources/icons/function.png.h"
#include "resources/icons/meta.png.h"
#include "resources/icons/module.png.h"
#include "resources/icons/type.png.h"
#include "resources/icons/typeref.png.h"
#include "resources/icons/variable.png.h"

static wxBitmap wxGetBitmapFromMemory(const unsigned char* data, int length) {
 wxMemoryInputStream strm(data, length);
 return wxBitmap(wxImage(strm, wxBITMAP_TYPE_PNG, -1), -1);
}

#define DEFINE_IMAGE_RESOURCE(symname, imgname) \
  wxBitmap Resources::Get ## symname ## Icon() { \
    static wxBitmap bm; \
    if (!bm.IsOk()) { \
      bm = wxGetBitmapFromMemory(imgname ## _png, sizeof(imgname ## _png)); \
    } \
    return bm; \
  }

DEFINE_IMAGE_RESOURCE(Collapse, collapse)
DEFINE_IMAGE_RESOURCE(Expand, expand)
DEFINE_IMAGE_RESOURCE(Module, module)
DEFINE_IMAGE_RESOURCE(Folder, folder)
DEFINE_IMAGE_RESOURCE(Type, type)
DEFINE_IMAGE_RESOURCE(Function, function)
DEFINE_IMAGE_RESOURCE(Variable, variable)
DEFINE_IMAGE_RESOURCE(Alias, alias)
DEFINE_IMAGE_RESOURCE(Constant, constant)
DEFINE_IMAGE_RESOURCE(Typeref, typeref)
DEFINE_IMAGE_RESOURCE(Meta, meta)
DEFINE_IMAGE_RESOURCE(Debug, debug)

wxImageList* Resources::GetTreeListIconList() {
  static wxImageList* icons = NULL;
  if (icons == NULL) {
    icons = new wxImageList(
        GetModuleIcon().GetWidth(),
        GetModuleIcon().GetHeight(),
        true, ICON_COUNT);
    // Important: Keep in sync with TreeListIconID enum.
    icons->Add(GetModuleIcon());
    icons->Add(GetFolderIcon());
    icons->Add(GetTypeIcon());
    icons->Add(GetFunctionIcon());
    icons->Add(GetVariableIcon());
    icons->Add(GetAliasIcon());
    icons->Add(GetConstantIcon());
    icons->Add(GetTyperefIcon());
    icons->Add(GetMetaIcon());
    icons->Add(GetDebugIcon());
  }
  return icons;
}

wxImageList* Resources::GetTreeListButtonList() {
  static wxImageList* icons = NULL;
  if (icons == NULL) {
    icons = new wxImageList(
        GetExpandIcon().GetWidth(),
        GetExpandIcon().GetHeight(),
        true, 4);
    icons->Add(GetExpandIcon());
    icons->Add(GetExpandIcon());
    icons->Add(GetCollapseIcon());
    icons->Add(GetCollapseIcon());
  }
  return icons;
}
