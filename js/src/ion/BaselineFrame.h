/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined(jsion_baseline_frame_h__) && defined(JS_ION)
#define jsion_baseline_frame_h__

#include "jscntxt.h"
#include "jscompartment.h"

#include "IonFrames.h"
#include "vm/Stack.h"

namespace js {
namespace ion {

// The stack looks like this, fp is the frame pointer:
//
// fp+y   arguments
// fp+x   IonJSFrameLayout (frame header)
// fp  => saved frame pointer
// fp-x   BaselineFrame
//        locals
//        stack values

// Eval frames
//
// Like js::StackFrame, every BaselineFrame is either a global frame
// or a function frame. Both global and function frames can optionally
// be "eval frames". The callee token for eval function frames is the
// enclosing function. BaselineFrame::evalScript_ stores the eval script
// itself.
class BaselineFrame
{
  public:
    enum Flags {
        // The frame has a valid return value. See also StackFrame::HAS_RVAL.
        HAS_RVAL         = 1 << 0,

        // Frame has blockChain_ set.
        HAS_BLOCKCHAIN   = 1 << 1,

        // A call object has been pushed on the scope chain.
        HAS_CALL_OBJ     = 1 << 2,

        // The evalPrev_ field has been initialized.
        HAS_EVAL_PREV    = 1 << 3,

        // Frame has an arguments object, argsObj_.
        HAS_ARGS_OBJ     = 1 << 4,

        // See StackFrame::PREV_UP_TO_DATE.
        PREV_UP_TO_DATE  = 1 << 5,

        // Eval frame, see the "eval frames" comment.
        EVAL             = 1 << 6,

        // Frame has hookData_ set.
        HAS_HOOK_DATA    = 1 << 7
    };

  protected: // Silence Clang warning about unused private fields.
    // We need to split the Value into 2 fields of 32 bits, otherwise the C++
    // compiler may add some padding between the fields.
    uint32_t loScratchValue_;
    uint32_t hiScratchValue_;
    uint32_t loReturnValue_;        // If HAS_RVAL, the frame's return value.
    uint32_t hiReturnValue_;
    uint32_t frameSize_;
    JSObject *scopeChain_;          // Scope chain (always initialized).
    StaticBlockObject *blockChain_; // If HAS_BLOCKCHAIN, the static block chain.
    JSScript *evalScript_;          // If isEvalFrame(), the current eval script.
    ArgumentsObject *argsObj_;      // If HAS_ARGS_OBJ, the arguments object.
    void *hookData_;                // If HAS_HOOK_DATA, debugger call hook data.
    uint32_t flags_;

    // In debug mode, evalPrev_ is a pointer to the previous frame
    // for eval frames. Only valid if HAS_EVAL_PREV is set.
    AbstractFramePtr evalPrev_;

#if JS_BITS_PER_WORD == 32
    uint32_t padding_;
#endif

  public:
    // Distance between the frame pointer and the frame header (return address).
    // This is the old frame pointer saved in the prologue.
    static const uint32_t FramePointerOffset = sizeof(void *);

    uint32_t frameSize() const {
        return frameSize_;
    }
    void setFrameSize(uint32_t frameSize) {
        frameSize_ = frameSize;
    }
    inline uint32_t *addressOfFrameSize() {
        return &frameSize_;
    }
    UnrootedObject scopeChain() const {
        return scopeChain_;
    }
    void setScopeChain(JSObject *scopeChain) {
        scopeChain_ = scopeChain;
    }
    inline JSObject **addressOfScopeChain() {
        return &scopeChain_;
    }

    inline Value *addressOfScratchValue() {
        return reinterpret_cast<Value *>(&loScratchValue_);
    }

    inline void pushOnScopeChain(ScopeObject &scope);
    inline void popOffScopeChain();

    CalleeToken calleeToken() const {
        uint8_t *pointer = (uint8_t *)this + Size() + offsetOfCalleeToken();
        return *(CalleeToken *)pointer;
    }
    UnrootedScript script() const {
        if (isEvalFrame())
            return evalScript();
        return ScriptFromCalleeToken(calleeToken());
    }
    JSFunction *fun() const {
        return CalleeTokenToFunction(calleeToken());
    }
    JSFunction *maybeFun() const {
        return isFunctionFrame() ? fun() : NULL;
    }
    UnrootedFunction callee() const {
        return CalleeTokenToFunction(calleeToken());
    }
    Value calleev() const {
        return ObjectValue(*callee());
    }
    size_t numValueSlots() const {
        size_t size = frameSize();

        JS_ASSERT(size >= BaselineFrame::FramePointerOffset + BaselineFrame::Size());
        size -= BaselineFrame::FramePointerOffset + BaselineFrame::Size();

        JS_ASSERT((size % sizeof(Value)) == 0);
        return size / sizeof(Value);
    }
    Value *valueSlot(size_t slot) const {
        JS_ASSERT(slot < numValueSlots());
        return (Value *)this - (slot + 1);
    }

    Value &unaliasedVar(unsigned i, MaybeCheckAliasing checkAliasing) const {
        JS_ASSERT_IF(checkAliasing, !script()->varIsAliased(i));
        JS_ASSERT(i < script()->nfixed);
        return *valueSlot(i);
    }

    Value &unaliasedFormal(unsigned i, MaybeCheckAliasing checkAliasing) const {
        JS_ASSERT(i < numFormalArgs());
        JS_ASSERT_IF(checkAliasing, !script()->argsObjAliasesFormals());
        JS_ASSERT_IF(checkAliasing, !script()->formalIsAliased(i));
        return formals()[i];
    }

    Value &unaliasedActual(unsigned i, MaybeCheckAliasing checkAliasing) const {
        JS_ASSERT(i < numActualArgs());
        JS_ASSERT_IF(checkAliasing, !script()->argsObjAliasesFormals());
        JS_ASSERT_IF(checkAliasing && i < numFormalArgs(), !script()->formalIsAliased(i));
        return actuals()[i];
    }

    Value &unaliasedLocal(unsigned i, MaybeCheckAliasing checkAliasing = CHECK_ALIASING) const {
#ifdef DEBUG
        CheckLocalUnaliased(checkAliasing, script(), maybeBlockChain(), i);
#endif
        return *valueSlot(i);
    }

    unsigned numActualArgs() const {
        return *(size_t *)(reinterpret_cast<const uint8_t *>(this) +
                             BaselineFrame::Size() +
                             offsetOfNumActualArgs());
    }
    unsigned numFormalArgs() const {
        return script()->function()->nargs;
    }
    Value &thisValue() const {
        return *(Value *)(reinterpret_cast<const uint8_t *>(this) +
                         BaselineFrame::Size() +
                         offsetOfThis());
    }
    Value *formals() const {
        return (Value *)(reinterpret_cast<const uint8_t *>(this) +
                         BaselineFrame::Size() +
                         offsetOfArg(0));
    }
    Value *actuals() const {
        return formals();
    }

    bool copyRawFrameSlots(AutoValueVector *vec) const;

    bool hasReturnValue() const {
        return flags_ & HAS_RVAL;
    }
    Value *returnValue() {
        return reinterpret_cast<Value *>(&loReturnValue_);
    }
    void setReturnValue(const Value &v) {
        flags_ |= HAS_RVAL;
        *returnValue() = v;
    }
    inline Value *addressOfReturnValue() {
        return reinterpret_cast<Value *>(&loReturnValue_);
    }

    bool hasBlockChain() const {
        return (flags_ & HAS_BLOCKCHAIN) && blockChain_;
    }
    StaticBlockObject &blockChain() const {
        JS_ASSERT(hasBlockChain());
        return *blockChain_;
    }
    StaticBlockObject *maybeBlockChain() const {
        return hasBlockChain() ? blockChain_ : NULL;
    }
    void setBlockChain(StaticBlockObject &block) {
        flags_ |= HAS_BLOCKCHAIN;
        blockChain_ = &block;
    }
    void setBlockChainNull() {
        JS_ASSERT(!hasBlockChain());
        blockChain_ = NULL;
    }
    StaticBlockObject **addressOfBlockChain() {
        return &blockChain_;
    }

    bool hasCallObj() const {
        return flags_ & HAS_CALL_OBJ;
    }

    CallObject &callObj() const {
        JS_ASSERT(hasCallObj());
        JS_ASSERT(fun()->isHeavyweight());

        JSObject *obj = scopeChain();
        while (!obj->isCall())
            obj = obj->enclosingScope();
        return obj->asCall();
    }

    void setFlags(uint32_t flags) {
        flags_ = flags;
    }
    uint32_t *addressOfFlags() {
        return &flags_;
    }

    inline bool pushBlock(JSContext *cx, Handle<StaticBlockObject *> block);
    inline void popBlock(JSContext *cx);

    bool strictEvalPrologue(JSContext *cx);
    bool heavyweightFunPrologue(JSContext *cx);

    void initArgsObj(ArgumentsObject &argsobj) {
        JS_ASSERT(script()->needsArgsObj());
        flags_ |= HAS_ARGS_OBJ;
        argsObj_ = &argsobj;
    }
    bool hasArgsObj() const {
        return flags_ & HAS_ARGS_OBJ;
    }
    ArgumentsObject &argsObj() const {
        JS_ASSERT(hasArgsObj());
        JS_ASSERT(script()->needsArgsObj());
        return *argsObj_;
    }

    bool prevUpToDate() const {
        return flags_ & PREV_UP_TO_DATE;
    }
    void setPrevUpToDate() {
        flags_ |= PREV_UP_TO_DATE;
    }

    JSScript *evalScript() const {
        JS_ASSERT(isEvalFrame());
        return evalScript_;
    }

    AbstractFramePtr evalPrev() const {
        JS_ASSERT(isEvalFrame());
        JS_ASSERT(flags_ & HAS_EVAL_PREV);
        return evalPrev_;
    }

    void initEvalPrev(JSContext *cx);

    bool hasHookData() const {
        return flags_ & HAS_HOOK_DATA;
    }

    void *maybeHookData() const {
        return hasHookData() ? hookData_ : NULL;
    }

    void setHookData(void *v) {
        hookData_ = v;
        flags_ |= HAS_HOOK_DATA;
    }

    void trace(JSTracer *trc);

    bool isFunctionFrame() const {
        return CalleeTokenIsFunction(calleeToken());
    }
    bool isGlobalFrame() const {
        return !CalleeTokenIsFunction(calleeToken());
    }
     bool isEvalFrame() const {
        return flags_ & EVAL;
    }
    bool isStrictEvalFrame() const {
        return isEvalFrame() && script()->strict;
    }
    bool isNonStrictEvalFrame() const {
        return isEvalFrame() && !script()->strict;
    }
    bool isDirectEvalFrame() const {
        return isEvalFrame() && script()->staticLevel > 0;
    }
    bool isNonStrictDirectEvalFrame() const {
        return isNonStrictEvalFrame() && isDirectEvalFrame();
    }
    bool isNonEvalFunctionFrame() const {
        return isFunctionFrame() && !isEvalFrame();
    }
    bool isDebuggerFrame() const {
        return false;
    }

    // Methods below are used by the compiler.
    static size_t offsetOfCalleeToken() {
        return FramePointerOffset + js::ion::IonJSFrameLayout::offsetOfCalleeToken();
    }
    static size_t offsetOfThis() {
        return FramePointerOffset + js::ion::IonJSFrameLayout::offsetOfThis();
    }
    static size_t offsetOfArg(size_t index) {
        return FramePointerOffset + js::ion::IonJSFrameLayout::offsetOfActualArg(index);
    }
    static size_t offsetOfNumActualArgs() {
        return FramePointerOffset + js::ion::IonJSFrameLayout::offsetOfNumActualArgs();
    }
    static size_t Size() {
        return sizeof(BaselineFrame);
    }

    // The reverseOffsetOf methods below compute the offset relative to the
    // frame's base pointer. Since the stack grows down, these offsets are
    // negative.
    static size_t reverseOffsetOfFrameSize() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, frameSize_);
    }
    static size_t reverseOffsetOfScratchValue() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, loScratchValue_);
    }
    static size_t reverseOffsetOfScopeChain() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, scopeChain_);
    }
    static size_t reverseOffsetOfBlockChain() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, blockChain_);
    }
    static size_t reverseOffsetOfArgsObj() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, argsObj_);
    }
    static size_t reverseOffsetOfFlags() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, flags_);
    }
    static size_t reverseOffsetOfEvalScript() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, evalScript_);
    }
    static size_t reverseOffsetOfReturnValue() {
        return -BaselineFrame::Size() + offsetof(BaselineFrame, loReturnValue_);
    }
    static size_t reverseOffsetOfLocal(size_t index) {
        return -BaselineFrame::Size() - (index + 1) * sizeof(Value);
    }
};

// Ensure the frame is 8-byte aligned (required on ARM).
JS_STATIC_ASSERT(((sizeof(BaselineFrame) + BaselineFrame::FramePointerOffset) % 8) == 0);

} // namespace ion
} // namespace js

#endif

