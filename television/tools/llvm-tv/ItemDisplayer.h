//===-- ItemDisplayer.h - Interface for tabbed-view components --*- C++ -*-===//
//
// The interface for tabbed views.
//
//===----------------------------------------------------------------------===//

#ifndef ITEMDISPLAYER_H
#define ITEMDISPLAYER_H

class TVTreeItemData;
class wxWindow;

class ItemDisplayer {
 public:
  virtual void displayItem (TVTreeItemData *item) = 0;

  /// getWindow - ItemDisplayers are responsible for explicitly keeping track of
  /// whatever "window" they are displaying the item into.
  ///
  virtual wxWindow *getWindow () = 0;

  /// getDisplayTitle - Return a title appropriate for when this
  /// ItemDisplayer displays the given item, or if item is NULL, any item.
  ///
  virtual std::string getDisplayTitle (TVTreeItemData *item) = 0;
};

#endif // ITEMDISPLAYER_H
