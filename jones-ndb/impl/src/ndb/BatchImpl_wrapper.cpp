/*
 Copyright (c) 2014, Oracle and/or its affiliates. All rights
 reserved.
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; version 2 of
 the License.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA
 */


#include <NdbApi.hpp>

#include "adapter_global.h"
#include "js_wrapper_macros.h"
#include "Record.h"
#include "NdbWrappers.h"
#include "BatchImpl.h"
#include "NativeMethodCall.h"
#include "NdbWrapperErrors.h"

using namespace v8;

V8WrapperFn getOperationError,
            tryImmediateStartTransaction,
            execute,
            executeAsynch,
            readBlobResults,
            BatchImpl_freeImpl;

class BatchImplEnvelopeClass : public Envelope {
public:
  BatchImplEnvelopeClass() : Envelope("BatchImpl") {
    EscapableHandleScope scope(v8::Isolate::GetCurrent());
    addMethod("tryImmediateStartTransaction", tryImmediateStartTransaction);
    addMethod("getOperationError", getOperationError);
    addMethod("execute", execute);
    addMethod("executeAsynch", executeAsynch);
    addMethod("readBlobResults", readBlobResults);
    addMethod("free", BatchImpl_freeImpl);
  }
};

BatchImplEnvelopeClass BatchImplEnvelope;

Handle<Value> BatchImpl_Wrapper(BatchImpl *set) {
  DEBUG_PRINT("BatchImpl wrapper");
  HandleScope scope;

  if(set) {
    Local<Object> jsobj = BatchImplEnvelope.newWrapper();
    wrapPointerInObject(set, BatchImplEnvelope, jsobj);
    freeFromGC(set, jsobj);
    return scope.Close(jsobj);
  }
  return Null(Isolate::GetCurrent());
}

Handle<Value> BatchImpl_Recycle(Handle<Object> oldWrapper, 
                                     BatchImpl * newSet) {
  DEBUG_PRINT("BatchImpl *Recycle*");
  assert(newSet);
  BatchImpl * oldSet = unwrapPointer<BatchImpl *>(oldWrapper);
  assert(oldSet == 0);
  wrapPointerInObject(newSet, BatchImplEnvelope, oldWrapper);
  return oldWrapper;
}

Persistent<Value> getWrappedObject(BatchImpl *set) {
  HandleScope scope;
  Local<Object> localObj = BatchImplEnvelope.newWrapper();
  wrapPointerInObject(set, BatchImplEnvelope, localObj);
  return Persistent<Value>::New(localObj);
}

void getOperationError(const Arguments & args) {
  DEBUG_MARKER(UDEB_DETAIL);
  EscapableHandleScope scope(args.GetIsolate());

  BatchImpl * set = unwrapPointer<BatchImpl *>(args.Holder());
  int n = args[0]->Int32Value();

  const NdbError * err = set->getError(n);

  if(err == 0) return True();
  if(err->code == 0) return Null();
  return scope.Close(NdbError_Wrapper(*err));
}

void tryImmediateStartTransaction(const Arguments &args) {
  BatchImpl * ctx = unwrapPointer<BatchImpl *>(args.Holder());
  return ctx->tryImmediateStartTransaction() ? True() : False();
}



/* ASYNC.
*/
/* Execute NdbTransaction.
   BatchImpl will close the transaction if exectype is not NoCommit;
   in this case, an extra call is made in the js main thread to register the
   transaction as closed.
*/
class TxExecuteAndCloseCall : 
  public NativeMethodCall_3_<int, BatchImpl, int, int, int> {
public:
  /* Constructor */
  TxExecuteAndCloseCall(const Arguments &args) : 
    NativeMethodCall_3_<int, BatchImpl, int, int, int>(
      & BatchImpl::execute, args) 
  {
    errorHandler = getNdbErrorIfLessThanZero;
  }
  void doAsyncCallback(Local<Object>);  
};                               

void TxExecuteAndCloseCall::doAsyncCallback(Local<Object> context) {
  if(arg0 != NdbTransaction::NoCommit) {
    native_obj->registerClosedTransaction();
  }
  NativeMethodCall_3_<int, BatchImpl, int, int, int>::doAsyncCallback(context);
}

void execute(const Arguments &args) {
  EscapableHandleScope scope(args.GetIsolate());
  REQUIRE_ARGS_LENGTH(4);
  TxExecuteAndCloseCall * ncallptr = new TxExecuteAndCloseCall(args);
  ncallptr->runAsync();
  args.GetReturnValue().SetUndefined();
}


/* IMMEDIATE.
*/
void executeAsynch(const Arguments &args) {
  EscapableHandleScope scope(args.GetIsolate());
  typedef NativeMethodCall_4_<int, BatchImpl,
                              int, int, int, Handle<Function> > MCALL;
  MCALL mcall(& BatchImpl::executeAsynch, args);
  mcall.run();
  args.GetReturnValue().Set(mcall.jsReturnVal());
}


void readBlobResults(const Arguments &args) {
  BatchImpl * set = unwrapPointer<BatchImpl *>(args.Holder());
  int n = args[0]->Int32Value();
  args.GetReturnValue().Set(set->getKeyOperation(n)->readBlobResults());
}


void BatchImpl_freeImpl(const Arguments &args) {
  BatchImpl * set = unwrapPointer<BatchImpl *>(args.Holder());
  delete set;
  set = 0;
  wrapPointerInObject(set, BatchImplEnvelope, args.Holder());
  args.GetReturnValue().SetUndefined();
}


