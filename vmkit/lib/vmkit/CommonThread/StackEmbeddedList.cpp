
#include "vmkit/StackEmbeddedList.h"
#include "vmkit/MethodInfo.h"

#include <iostream>

#if 0
// #if EMBEDDED_LIST_IN_CALL_STACK

namespace vmkit {

bool StackEmbeddedListWalker::findStackFrameOfEmbeddedNode()
{
	/* Consider the following x86 stack frame:

|-| (last method called)
| |     ...
| | BP(i-2)
| |  |  ret addr(i-2)
| |  |  args(i-2)
|a|  |  saved regs(i-1)
|d|  V  locals(i-1)
|d| BP(i-1)
|r|  |  ret addr(i-1)
|e|  |  args(i-1)
|s|  |  saved regs(i)
|s|  V  locals(i)
| | BP(i  )
| |    ret addr(i)
| |    args(i)
| |    ...
|+| (first caller method)

    BP := call frame address (base pointer).
    The intended caller linked list nodes are generated in the locals area of
    the caller method frame.

    In order to find the given node in "locals(i)", we need to know where
    "locals(i)" is located = [BP(i-1), BP(i)].
    If the node is found in "locals(i)", this means that the frame "i-1" is the
    caller of the intended method, so the intended method frame itself is "i-2".
    This is why we need to keep track of the last two stack frames when walking
    the stack.
    Because some frames are Java method stub frames, we need to know which
    frame they call (the actual called Java method). Thus we need to keep the
    last three stack frames.
    Because we need to preserve those 4 stack frames, and because we might go
    backward in the stack queue by at most 3 elements, we need to preserve the
    stack queue. This is the purpose of walkedStack.
	*/

	word_t nodeStart = (word_t)_current_node;
	word_t nodeEnd = nodeStart + sizeof(*_current_node);

	auto callFrameStart = _walked_stack.empty() ?
		_stack_walker.callFrameAddress :
		_walked_stack.front().callFrameAddress;

	for (; _stack_walker.get() != nullptr;
		_walked_stack.push_front(_stack_walker), ++_stack_walker)
	{
		auto callFrameEnd = _stack_walker.callFrameAddress;

		if ((nodeStart >= callFrameStart) && (nodeEnd <= callFrameEnd)) {
			// Found the node, go backward two frames, if possible.
			switch (_walked_stack.size()) {
			case 0:
				// We actually have an incomplete stack frame (VMKit bug).
				// We don't have access neither to the called frame nor the
				// called frame.
				// Report the caller of the caller frame (basically no op).
				break;

			case 1:
				// We actually have an incomplete stack frame (VMKit bug).
				// We don't have access to the called frame.
				// Report the caller frame.
				_stack_walker = _walked_stack.front();
				_walked_stack.pop_front();
				break;

			default:	// _walked_stack.size() >= 2
				_walked_stack.pop_front();			// Remove the frame -1.
				_stack_walker = _walked_stack.front();	// Get the frame -2.
				_walked_stack.pop_front();			// Remove the frame -2.

				// If the frame is a stub, then return the actual Java method.
				auto FI = _stack_walker.get();
				if (!FI->Metadata && (FI->FrameSize > 0) &&
					!_walked_stack.empty())
				{
					_stack_walker = _walked_stack.front();	// Get the frame -3.
					_walked_stack.pop_front();			// Remove the frame -3.
				}
			}

			return true;
		}

		callFrameStart = callFrameEnd;
	}

	return false;	// Node not found
}

bool StackEmbeddedListWalker::next()
{
	if (!_current_node) {	// Initial _current_node
		_current_node = _stack_walker.thread->
			stackEmbeddedListHead[StackEmbeddedListChargedTier];
	} else {
		_current_node = _current_node->callerNode;	// Get the next node
	}

	while (_current_node != nullptr) {	// Find the caller Java method frame.
		if (findStackFrameOfEmbeddedNode())
			break;

		_current_node = _current_node->callerNode;
	}

	return _current_node != nullptr;
}

void StackEmbeddedListNode::dump() const
{
	std::cout << "tierID=" << reinterpret_cast<uint32_t>(data[0]) << std::endl;
	if (!callerNode) return;
	callerNode->dump();
}

}

#endif
