#include "CodeViewer.h"
#include "TVApplication.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/Value.h"
#include <sstream>
using namespace llvm;

const std::string& TVCodeItem::getLabel() {
  if (!Val)
    label = "<badref>";
  else if (label != "")
    return label;
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
    assert(0 && "Unhandled Value type!");
  

  return label;
}

void TVCodeViewer::refreshView() {
  // Clear out the list and then re-add all the items.
  int index = 0;
  ClearAll();
  for (Items::iterator i = itemList.begin(), e = itemList.end(); i != e; ++i) {
    InsertItem (index, (*i)->getLabel().c_str());
    ++index;
  }
}


TVCodeViewer::TVCodeViewer(wxWindow *_parent, llvm::Function *F)
  : wxListCtrl(_parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_LIST) {
  for (Function::iterator BB = F->begin(), BBe = F->end(); BB != BBe; ++BB) {
    itemList.push_back(new TVCodeItem(BB));
    for (BasicBlock::iterator I = BB->begin(), Ie = BB->end(); I != Ie; ++I)
      itemList.push_back(new TVCodeItem(I));
  }
  refreshView();
}

// CodeViewFrame implementation
//
bool CodeViewFrame::OnClose (wxCloseEvent &event) {
  myApp->GoodbyeFrom (this);
  Destroy();
  return true;
}

BEGIN_EVENT_TABLE (CodeViewFrame, wxFrame)
  EVT_CLOSE (CodeViewFrame::OnClose)
END_EVENT_TABLE ()
