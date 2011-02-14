/*
 * Resources.h
 */

#ifndef LLBROWSE_RESOURCES_H
#define LLBROWSE_RESOURCES_H

#ifndef CONFIG_H
#include "config.h"
#endif

#ifndef _WX_WX_H_
#include "wx/wx.h"
#endif

#ifndef _WX_IMAGELIST_H_
#include <wx/imaglist.h>
#endif

class Resources {
public:
  enum TreeListIconID {
    ICON_MODULE,
    ICON_FOLDER,
    ICON_TYPE,
    ICON_FUNCTION,
    ICON_VARIABLE,
    ICON_ALIAS,
    ICON_CONSTANT,
    ICON_TYPEREF,
    ICON_META,
    ICON_DEBUG,

    ICON_COUNT
  };

  // Tree list buttons
  static wxBitmap GetCollapseIcon();
  static wxBitmap GetExpandIcon();

  // Tree list icons
  static wxBitmap GetModuleIcon();
  static wxBitmap GetFolderIcon();
  static wxBitmap GetTypeIcon();
  static wxBitmap GetFunctionIcon();
  static wxBitmap GetVariableIcon();
  static wxBitmap GetAliasIcon();
  static wxBitmap GetConstantIcon();
  static wxBitmap GetTyperefIcon();
  static wxBitmap GetMetaIcon();
  static wxBitmap GetDebugIcon();

  // Image lists
  static wxImageList * GetTreeListIconList();
  static wxImageList * GetTreeListButtonList();
};

#endif // LLBROWSE_RESOURCES_H
