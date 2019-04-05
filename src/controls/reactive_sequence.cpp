/* Copyright (C) 2019 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree_cpp/controls/reactive_sequence.h"
#include <thread> // *TODO* REMOVE AFTER DEBUG

using std::chrono::milliseconds;

namespace BT
{

NodeStatus ReactiveSequence::tick()
{
    size_t success_count = 0;
    size_t running_count = 0;

    for (size_t index = 0; index < childrenCount(); index++)
    {
        TreeNode* current_child_node = children_nodes_[index];
        NodeStatus child_status = NodeStatus::IDLE;

        if (current_child_node->type() != NodeType::ACTION_ASYNC)
        {
            child_status = current_child_node->executeTick();
        }
        else
        {
            if (current_child_node->status() != NodeStatus::RUNNING)
            {
                // if not running already, tick it
                current_child_node->executeTick();
                do
                {
                    child_status = current_child_node->status();
                } while (child_status == NodeStatus::IDLE);
            }
            else
            {
                child_status = NodeStatus::RUNNING;
            }
        }

        switch (child_status)
        {
        case NodeStatus::RUNNING:
        {
            running_count++;
            haltChildren(index+1);
            return NodeStatus::RUNNING;
        }

        case NodeStatus::FAILURE:
        {
            haltChildren(index+1);
            return NodeStatus::FAILURE;
        }
        case NodeStatus::SUCCESS:
        {
            success_count++;
        }break;

        case NodeStatus::IDLE:
        {
            throw LogicError("A child node must never return IDLE");
        }
        }   // end switch
    } //end for

    if( success_count == childrenCount())
    {
        haltChildren(0);
        return NodeStatus::SUCCESS;
    }
    return NodeStatus::RUNNING;
}


}
