#include "CodeViewer.h"
#include "TVApplication.h"
#include "TVFrame.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/Value.h"
#include "Support/StringExtras.h"
#include <wx/listctrl.h>
#include <map>
#include <sstream>
using namespace llvm;

void TVCodeItem::SetLabel() {
  std::string label;
  if (!Val)
    label = "<badref>";
  else if (BasicBlock *BB = dyn_cast<BasicBlock>(Val))
    label = BB->getName() + ":";
  else if (Instruction *I = dyn_cast<Instruction>(Val)) {
    std::ostringstream out;
    I->print(out);
    label = out.str();
    // Post-processing for more attractive display
    for (unsigned i = 0; i != label.length(); ++i)
      if (label[i] == '\n') {                            // \n => space
        label[i] = ' ';
      } else if (label[i] == '\t') {                     // \t => 2 spaces
        label[i] = ' ';
        label.insert(label.begin()+i+1, ' ');
      } else if (label[i] == ';') {                      // Delete comments!
        unsigned Idx = label.find('\n', i+1);            // Find end of line
        label.erase(label.begin()+i, label.begin()+Idx);
        --i;
      }

  } else
    label = "<invalid value>";

  SetText(label.c_str());
}

//===----------------------------------------------------------------------===//

void TVCodeListCtrl::refreshView() {
  // Hide the list while rewriting it from scratch to speed up rendering
  Hide();

  // Clear out the list and then re-add all the items.
  long index = 0;
  ClearAll();
  for (Items::iterator i = itemList.begin(), e = itemList.end(); i != e; ++i) {
    InsertItem(index, (**i).m_text.c_str());
    ItemToIndex[*i] = index;
    ++index;
  }

  Show();
}

template<class T> void wipe (T x) { delete x; }

void TVCodeListCtrl::SetFunction (Function *F) {
  // Empty out the code list.
  if (!itemList.empty ())
    for_each (itemList.begin (), itemList.end (), wipe<TVCodeItem *>);
  itemList.clear ();
  ValueToItem.clear ();

  // Populate it with BasicBlocks and Instructions from F.
  for (Function::iterator BB = F->begin(), BBe = F->end(); BB != BBe; ++BB) {
    itemList.push_back(new TVCodeItem(BB));
    for (BasicBlock::iterator I = BB->begin(), Ie = BB->end(); I != Ie; ++I) {
      TVCodeItem *TCI = new TVCodeItem(I);
      itemList.push_back(TCI);
      ValueToItem[I] = TCI;
    }
  }

  // Since the function changed, we had better make sure that the list control
  // matches what's in the code list.
  refreshView ();
}

TVCodeListCtrl::TVCodeListCtrl(wxWindow *_parent, llvm::Function *F)
  : wxListCtrl(_parent, LLVM_TV_CODEVIEW_LIST, wxDefaultPosition, wxDefaultSize,
               wxLC_LIST) {
  SetFunction (F);
}

void TVCodeListCtrl::OnItemSelected(wxListEvent &event) {
  Value *V = itemList[event.GetIndex ()]->getValue();
  if (!V) return;

  // Highlight uses
  for (User::use_iterator u = V->use_begin(), e = V->use_end(); u != e; ++u) {
    TVCodeItem *item = ValueToItem[*u];
    item->m_itemId = ItemToIndex[item];
    item->SetTextColour(*wxRED);
    wxFont Font = item->GetFont();
    Font.SetWeight(wxBOLD);
    item->SetFont(Font);
    SetItem(*item);
  }
}

void TVCodeListCtrl::OnItemDeselected(wxListEvent &event) {
  Value *V = itemList[event.GetIndex ()]->getValue();
  if (!V) return;

  // Set uses back to normal
  for (User::use_iterator u = V->use_begin(), e = V->use_end(); u != e; ++u) {
    TVCodeItem *item = ValueToItem[*u];
    item->m_itemId = ItemToIndex[item];
    item->SetTextColour(*wxBLACK);
    wxFont Font = item->GetFont();
    Font.SetWeight(wxNORMAL);
    item->SetFont(Font);
    SetItem(*item);
  }
}

BEGIN_EVENT_TABLE (TVCodeListCtrl, wxListCtrl)
  EVT_LIST_ITEM_SELECTED(LLVM_TV_CODEVIEW_LIST,
                         TVCodeListCtrl::OnItemSelected)
  EVT_LIST_ITEM_DESELECTED(LLVM_TV_CODEVIEW_LIST,
                           TVCodeListCtrl::OnItemDeselected)
END_EVENT_TABLE ()

//===----------------------------------------------------------------------===//

// CodeViewFrame implementation
//
void CodeViewFrame::OnClose (wxCloseEvent &event) {
  myApp->GoodbyeFrom (this);
  Destroy();
}

BEGIN_EVENT_TABLE (CodeViewFrame, wxFrame)
  EVT_CLOSE (CodeViewFrame::OnClose)
END_EVENT_TABLE ()
