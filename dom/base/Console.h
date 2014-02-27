/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_Console_h
#define mozilla_dom_Console_h

#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/UnionConversions.h"
#include "mozilla/ErrorResult.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDataHashtable.h"
#include "nsHashKeys.h"
#include "nsITimer.h"
#include "nsWrapperCache.h"

class nsIConsoleAPIStorage;

namespace mozilla {
namespace dom {

class ConsoleCallData;

class Console MOZ_FINAL : public nsITimerCallback
                        , public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(Console)
  NS_DECL_NSITIMERCALLBACK

  Console(nsPIDOMWindow* aWindow);
  ~Console();

  // WebIDL methods
  nsISupports* GetParentObject() const
  {
    return mWindow;
  }

  virtual JSObject*
  WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope) MOZ_OVERRIDE;

  void
  Log(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Info(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Warn(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Error(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Exception(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Debug(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Trace(JSContext* aCx);

  void
  Dir(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Group(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  GroupCollapsed(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  GroupEnd(JSContext* aCx, const Sequence<JS::Value>& aData);

  void
  Time(JSContext* aCx, const JS::Handle<JS::Value> aTime);

  void
  TimeEnd(JSContext* aCx, const JS::Handle<JS::Value> aTime);

  void
  Profile(JSContext* aCx, const Sequence<JS::Value>& aData,
          ErrorResult& aRv);

  void
  ProfileEnd(JSContext* aCx, const Sequence<JS::Value>& aData,
             ErrorResult& aRv);

  void
  Assert(JSContext* aCx, bool aCondition, const Sequence<JS::Value>& aData);

  void
  __noSuchMethod__();

private:
  enum MethodName
  {
    MethodLog,
    MethodInfo,
    MethodWarn,
    MethodError,
    MethodException,
    MethodDebug,
    MethodTrace,
    MethodDir,
    MethodGroup,
    MethodGroupCollapsed,
    MethodGroupEnd,
    MethodTime,
    MethodTimeEnd,
    MethodAssert
  };

  void
  Method(JSContext* aCx, MethodName aName, const nsAString& aString,
         const Sequence<JS::Value>& aData);

  void
  ProcessCallData(ConsoleCallData& aData);

  // If the first JS::Value of the array is a string, this method uses it to
  // format a string. The supported sequences are:
  //   %s    - string
  //   %d,%i - integer
  //   %f    - double
  //   %o    - a JS object.
  // The output is an array where any object is a separated item, the rest is
  // unified in a format string.
  // Example if the input is:
  //   "string: %s, integer: %d, object: %o, double: %d", 's', 1, window, 0.9
  // The output will be:
  //   [ "string: s, integer: 1, object: ", window, ", double: 0.9" ]
  void
  ProcessArguments(JSContext* aCx, const nsTArray<JS::Heap<JS::Value>>& aData,
                   Sequence<JS::Value>& aSequence);

  void
  MakeFormatString(nsCString& aFormat, int32_t aInteger, int32_t aMantissa,
                   char aCh);

  // Stringify and Concat all the JS::Value in a single string using ' ' as
  // separator.
  void
  ComposeGroupName(JSContext* aCx, const nsTArray<JS::Heap<JS::Value>>& aData,
                   nsAString& aName);

  JS::Value
  StartTimer(JSContext* aCx, const JS::Value& aName,
             DOMHighResTimeStamp aTimestamp);

  JS::Value
  StopTimer(JSContext* aCx, const JS::Value& aName,
            DOMHighResTimeStamp aTimestamp);

  // The method populates a Sequence from an array of JS::Value.
  void
  ArgumentsToValueList(const nsTArray<JS::Heap<JS::Value>>& aData,
                       Sequence<JS::Value>& aSequence);

  void
  ProfileMethod(JSContext* aCx, const nsAString& aAction,
                const Sequence<JS::Value>& aData,
                ErrorResult& aRv);

  nsCOMPtr<nsPIDOMWindow> mWindow;
  nsCOMPtr<nsITimer> mTimer;
  nsCOMPtr<nsIConsoleAPIStorage> mStorage;

  nsTArray<ConsoleCallData> mQueuedCalls;
  nsDataHashtable<nsStringHashKey, DOMHighResTimeStamp> mTimerRegistry;

  uint64_t mOuterID;
  uint64_t mInnerID;

  friend class ConsoleCallData;
};

} // dom namespace
} // mozilla namespace

#endif /* mozilla_dom_Console_h */
