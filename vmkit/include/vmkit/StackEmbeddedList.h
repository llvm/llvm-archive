
#ifndef STACK_EMBEDDED_LIST_H_
#define STACK_EMBEDDED_LIST_H_

#include "vmkit/System.h"
#include "vmkit/StackWalker.h"

#include <list>

#if EMBEDDED_LIST_IN_CALL_STACK

namespace vmkit {

class Thread;

enum StackEmbeddedListID {
	StackEmbeddedListChargedTier = 0,

	StackEmbeddedListNodeCountPerThread
};

struct StackEmbeddedListNode {
	StackEmbeddedListNode*	callerNode;
	word_t					data[1];

	void dump() const __attribute__((noinline));
};

}

/*
class StackEmbeddedListWalker
{
protected:
	const StackEmbeddedListNode*	_current_node;
	StackWalker						_stack_walker;
	std::list<StackWalker>			_walked_stack;

public:
	StackEmbeddedListWalker(Thread* thread) :
		_current_node(nullptr), _stack_walker(thread) {}
	virtual ~StackEmbeddedListWalker() {}

	bool findStackFrameOfEmbeddedNode();
	bool next();

	StackWalker& getStackWalker() {return _stack_walker;}
	uint32_t getChargedTierID() const {
		return (!_current_node)
			? 0 : reinterpret_cast<uint32_t>(_current_node->data[0]);
	}
};

extern "C" void pushChargedTierNode(vmkit::Thread* thread,
	vmkit::StackEmbeddedListNode* node);

extern "C" void popChargedTierNode(vmkit::Thread* thread,
	vmkit::StackEmbeddedListNode* node);
*/

#endif

#endif
