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

void TVCodeViewer::refreshView() {
  // Hide the list while rewriting it from scratch to speed up rendering
  this->Hide();

  // Clear out the list and then re-add all the items.
  unsigned index = 0;
  ClearAll();
  for (Items::iterator i = itemList.begin(), e = itemList.end(); i != e; ++i) {
    InsertItem(index, (**i).m_text.c_str());
    ItemToIndex[*i] = index;
    ++index;
  }

  this->Show();
}


TVCodeViewer::TVCodeViewer(wxWindow *_parent, llvm::Function *F)
  : wxListCtrl(_parent, LLVM_TV_CODEVIEW_LIST, wxDefaultPosition, wxDefaultSize,
               wxLC_LIST) {
  for (Function::iterator BB = F->begin(), BBe = F->end(); BB != BBe; ++BB) {
    itemList.push_back(new TVCodeItem(BB));
    for (BasicBlock::iterator I = BB->begin(), Ie = BB->end(); I != Ie; ++I) {
      TVCodeItem *TCI = new TVCodeItem(I);
      itemList.push_back(TCI);
      ValueToItem[I] = TCI;
    }
  }
  refreshView();
}

void TVCodeViewer::OnItemActivated(wxListEvent &event) {
  //std::cerr << "activated item: " << utostr((unsigned)event.GetIndex()) << "\n";
}

void TVCodeViewer::OnItemSelected(wxListEvent &event) {
  //std::cerr << "selected item: " << utostr((unsigned)event.GetIndex()) << "\n";
  long int index = event.GetIndex();
  // Highlight uses
  Value *V = itemList[index]->getValue();
  if (!V) return;

  for (User::use_iterator u = V->use_begin(), e = V->use_end(); u != e; ++u) {
    //wxListItem &item = GetItem(event.GetIndex());
    //item.SetTextColor(*wxRED);
    //item.SetFont(*wxBOLD_FONT);
    TVCodeItem *item = ValueToItem[*u];
    item->m_itemId = ItemToIndex[item];
    item->SetTextColour(*wxRED);
    wxFont Font = item->GetFont();
    Font.SetWeight(wxBOLD);
    item->SetFont(Font);
    SetItem(*item);
  }
}

void TVCodeViewer::OnItemDeselected(wxListEvent &event) {
  //std::cerr << "deselected item: " << utostr((unsigned)event.GetIndex()) << "\n";
  long int index = event.GetIndex();
  // Set uses back to normal
  Value *V = itemList[index]->getValue();
  if (!V) return;

  for (User::use_iterator u = V->use_begin(), e = V->use_end(); u != e; ++u) {
    //wxListItem &item = GetItem(event.GetIndex());
    //item.SetTextColor(*wxRED);
    //item.SetFont(*wxBOLD_FONT);
    TVCodeItem *item = ValueToItem[*u];
    item->m_itemId = ItemToIndex[item];
    item->SetTextColour(*wxBLACK);
    wxFont Font = item->GetFont();
    Font.SetWeight(wxNORMAL);
    item->SetFont(Font);
    SetItem(*item);
  }
}

BEGIN_EVENT_TABLE (TVCodeViewer, wxListCtrl)
  EVT_LIST_ITEM_ACTIVATED(LLVM_TV_CODEVIEW_LIST,
                          TVCodeViewer::OnItemActivated)
  EVT_LIST_ITEM_SELECTED(LLVM_TV_CODEVIEW_LIST,
                         TVCodeViewer::OnItemSelected)
  EVT_LIST_ITEM_DESELECTED(LLVM_TV_CODEVIEW_LIST,
                           TVCodeViewer::OnItemDeselected)
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
