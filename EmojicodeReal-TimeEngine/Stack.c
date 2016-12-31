//
//  Stack.c
//  Emojicode
//
//  Created by Theo Weidmann on 02.08.15.
//  Copyright (c) 2015 Theo Weidmann. All rights reserved.
//

#include "Emojicode.h"
#include <string.h>

Something* stackReserveFrame(Something this, int size, Thread *thread) {
    StackFrame *sf = (StackFrame *)(thread->futureStack - (sizeof(StackFrame) + sizeof(Something) * size));
    if ((Byte *)sf < thread->stackLimit) {
        error("Your program triggerd a stack overflow!");
    }
    
    memset((Byte *)sf + sizeof(StackFrame), 0, sizeof(Something) * size);
    
    sf->thisContext = this;
    sf->size = size;
    sf->returnPointer = thread->stack;
    sf->returnFutureStack = thread->futureStack;
    
    thread->futureStack = (Byte *)sf;
    
    return (Something *)(((Byte *)sf) + sizeof(StackFrame));
}

void stackPushReservedFrame(Thread *thread){
    thread->stack = thread->futureStack;
}

void stackPush(Something this, int frameSize, uint8_t argCount, Thread *thread) {
    Something *t = stackReserveFrame(this, frameSize, thread);
    
    for (int i = 0, index = 0; i < argCount; i++) {
        EmojicodeCoin copySize = consumeCoin(thread);
        produce(consumeCoin(thread), thread, t + index);
        index += copySize;
    }
    
    stackPushReservedFrame(thread);
}

void stackPop(Thread *thread) {
    thread->futureStack = ((StackFrame *)thread->stack)->returnFutureStack;
    thread->stack = ((StackFrame *)thread->stack)->returnPointer;
}

StackState storeStackState(Thread *thread) {
    StackState s = {thread->futureStack, thread->stack};
    return s;
}

void restoreStackState(StackState s, Thread *thread) {
    thread->futureStack = s.futureStack;
    thread->stack = s.stack;
}

Something stackGetVariable(int index, Thread *thread) {
    return *stackVariableDestination(index, thread);
}

Something* stackVariableDestination(int index, Thread *thread) {
    return (Something *)(thread->stack + sizeof(StackFrame) + sizeof(Something) * index);
}

Object* stackGetThisObject(Thread *thread) {
    return ((StackFrame *)thread->stack)->thisContext.object;
}

Something stackGetThisContext(Thread *thread) {
    return ((StackFrame *)thread->stack)->thisContext;
}

void stackMark(Thread *thread){
    for (StackFrame *stackFrame = (StackFrame *)thread->futureStack; (Byte *)stackFrame < thread->stackBottom; stackFrame = stackFrame->returnFutureStack) {
        for (uint8_t i = 0; i < stackFrame->size; i++) {
            Something *s = (Something *)(((Byte *)stackFrame) + sizeof(StackFrame) + sizeof(Something) * i);
            if (isRealObject(*s)) {
                mark(&s->object);
            }
        }
        if (isRealObject(stackFrame->thisContext)) {
            mark(&stackFrame->thisContext.object);
        }
    }
}
