#ifndef ITEMDISPLAYER_H
#define ITEMDISPLAYER_H

class TVTreeItemData;

class ItemDisplayer {
 public:
  virtual void displayItem (TVTreeItemData *item) = 0;
};

#endif // ITEMDISPLAYER_H
