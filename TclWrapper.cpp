/*
*	 The MIT License (MIT)
*
*	 Copyright (c) 2015 Alisa Dolinsky
*
*	 Permission is hereby granted, free of charge, to any person obtaining a copy
*	 of this software and associated documentation files (the "Software"), to deal
*	 in the Software without restriction, including without limitation the rights
*	 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	 copies of the Software, and to permit persons to whom the Software is
*	 furnished to do so, subject to the following conditions:
*
*	 The above copyright notice and this permission notice shall be included in all
*	 copies or substantial portions of the Software.
*
*	 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	 SOFTWARE.
*/

#include "EmbedTcl.h"
#include "TclWrapper.hpp"

TclWrapper::TclWrapper(bool bootstrapSuccess = false, uint32 _id = 0) {
	if(bootstrapSuccess) {
		interpreter = _Tcl_CreateInterp();
		registerId(_id);
		UE_LOG(LogClass, Log, TEXT("Allocated %i bytes for a Tcl interpreter"), interpreterSize)
	} else {
		interpreter = nullptr;
		UE_LOG(LogClass, Log, TEXT("Failed to Allocate a Tcl interpreter"))
	}
}

TclWrapper::~TclWrapper() {
	// free interpreter here!
}

void* TclWrapper::handle = nullptr;
size_t TclWrapper::interpreterSize = 0;
const char* TclWrapper::__id__ = "__id__";

_Tcl_CreateInterpProto TclWrapper::_Tcl_CreateInterp;

bool TclWrapper::bootstrapSuccess() {
	return !(handle == nullptr || interpreter == nullptr);
}

_Tcl_EvalProto TclWrapper::_Tcl_Eval;
int TclWrapper::eval(const char* code) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else { return _Tcl_Eval(interpreter, code); }
}

_Tcl_CreateObjCommandProto TclWrapper::_Tcl_CreateObjCommand;
int TclWrapper::registerFunction(const char* fname, Tcl_ObjCmdProc* f, ClientData clientData = (ClientData)nullptr, Tcl_CmdDeleteProc* deleteCallback = (Tcl_CmdDeleteProc*)nullptr) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; } else {
		_Tcl_CreateObjCommand(interpreter, fname, f, clientData, deleteCallback);
		return TCL_OK;
	}
}

template<typename cls, typename returnType, typename ...paramTypes> int TclWrapper::bind(FString name, cls* self) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; } else {
		auto wrapper = [](ClientData clientData, Tcl_Interp* interpreter, int numberOfArgs, Tcl_Obj* const args[]) -> int {
			if ((numberOfArgs < 3) || (numberOfArgs > 3)) {
				UE_LOG(LogClass, Log, TEXT("Tcl: wrong # of args for the proc -> moveToLoc x y"))
				return TCL_ERROR;
			}
			auto arg1 = const_cast<Tcl_Obj*>(args[1]);
			double _arg1;
			if (TclWrapper::toDouble(interpreter, arg1, &_arg1) == TCL_ERROR) return TCL_ERROR;
			auto arg2 = const_cast<Tcl_Obj*>(args[2]);
			double _arg2;
			if (TclWrapper::toDouble(interpreter, arg2, &_arg2) == TCL_ERROR) return TCL_ERROR;

			auto self = (cls*)clientData;

			TBaseDelegate<returnType> WriteToLogDelegate;
			WriteToLogDelegate.BindUFunction(self, "hello");
			WriteToLogDelegate.ExecuteIfBound();

			TBaseDelegate<returnType, paramTypes...> WriteToLogDelegate2;
			WriteToLogDelegate2.BindUFunction(self, "hello2");
			WriteToLogDelegate2.ExecuteIfBound("hello!");

			return TCL_OK;
		};
		const char* fname = TCHAR_TO_ANSI(*name);
		_Tcl_CreateObjCommand(interpreter, fname, wrapper, (ClientData)self, (Tcl_CmdDeleteProc*)nullptr);
		return TCL_OK;
	}
}

_Tcl_ObjSetVar2Proto TclWrapper::_Tcl_ObjSetVar2;
int TclWrapper::define(Tcl_Obj* location, Tcl_Obj* scope, Tcl_Obj* val, int flags) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else {
		_Tcl_ObjSetVar2(interpreter, location, scope, val, flags);
		return TCL_OK;
	}
}

_Tcl_ObjGetVar2Proto TclWrapper::_Tcl_ObjGetVar2;
int TclWrapper::fetch(Tcl_Obj* location, Tcl_Obj* scope, Tcl_Obj** val, int flags) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else {
		*val = _Tcl_ObjGetVar2(interpreter, location, scope, flags);
		return TCL_OK;
	}
}

int TclWrapper::registerId(uint32 _id) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else {
		Tcl_Obj* __id__;
		TclWrapper::newString(&__id__, TclWrapper::__id__);
		Tcl_Obj* id;
		TclWrapper::newLong(&id, (long)_id);
		_Tcl_ObjSetVar2(interpreter, __id__, nullptr, id, TCL_GLOBAL_ONLY);
		return TCL_OK;
	}
}

int TclWrapper::id(Tcl_Interp* interpreter, uint32* _id) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else {
		Tcl_Obj* __id__;
		TclWrapper::newString(&__id__, TclWrapper::__id__);
		auto _idObj = _Tcl_ObjGetVar2(interpreter, __id__, nullptr, TCL_GLOBAL_ONLY);
		long __id;
		auto status = toLong(interpreter, _idObj, &__id);
		*_id = (uint32)__id;
		return status;
	}
}
uint32 TclWrapper::id(Tcl_Interp* interpreter) {
	uint32 _id;
	TclWrapper::id(interpreter, &_id);
	return _id;
}

int TclWrapper::id(uint32* _id) {
	return id(interpreter, _id);
}
uint32 TclWrapper::id() {
	uint32 _id;
	id(interpreter, &_id);
	return _id;
}

_Tcl_GetObjResultProto TclWrapper::_Tcl_GetObjResult;
int TclWrapper::getResult(Tcl_Interp* interpreter, Tcl_Obj** obj) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else {
		*obj = _Tcl_GetObjResult(interpreter);
		return TCL_OK;
	}
}

_Tcl_NewStringObjProto TclWrapper::_Tcl_NewStringObj;
int TclWrapper::newString(Tcl_Obj** obj, const char* val) {
	if (handle == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else {
		*obj = _Tcl_NewStringObj(val, -1);
		return TCL_OK;
	}
}

_Tcl_NewLongObjProto TclWrapper::_Tcl_NewLongObj;
int TclWrapper::newLong(Tcl_Obj** obj, long val) {
	if (handle == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else {
		*obj = _Tcl_NewLongObj(val);
		return TCL_OK;
	}
}

_Tcl_SetIntObjProto TclWrapper::_Tcl_SetIntObj;
int TclWrapper::setInt(Tcl_Obj* obj, int val) {
	if (handle == nullptr) { return _TCL_BOOTSTRAP_FAIL_; } else {
		_Tcl_SetIntObj(obj, val);
		return TCL_OK;
	}
}

_Tcl_SetStringObjProto TclWrapper::_Tcl_SetStringObj;
int TclWrapper::setString(Tcl_Obj* obj, const char* str, int len) {
	if (handle == nullptr) { return _TCL_BOOTSTRAP_FAIL_; } else {
		_Tcl_SetStringObj(obj, str, len);
		return TCL_OK;
	}
}

_Tcl_GetIntFromObjProto TclWrapper::_Tcl_GetIntFromObj;
int TclWrapper::toInt(Tcl_Interp* interpreter, Tcl_Obj* obj, int* val) {
	if (handle == nullptr || interpreter == nullptr ) { return _TCL_BOOTSTRAP_FAIL_; }
	else { return _Tcl_GetIntFromObj(interpreter, obj, val); }
}
int TclWrapper::toInt(Tcl_Obj* obj, int* val) { return toInt(interpreter, obj, val); }

_Tcl_GetLongFromObjProto TclWrapper::_Tcl_GetLongFromObj;
int TclWrapper::toLong(Tcl_Interp* interpreter, Tcl_Obj* obj, long* val) {
	if (handle == nullptr || interpreter == nullptr ) { return _TCL_BOOTSTRAP_FAIL_; }
	else { return _Tcl_GetLongFromObj(interpreter, obj, val); }
}
int TclWrapper::toLong(Tcl_Obj* obj, long* val) { return toLong(interpreter, obj, val); }

_Tcl_GetDoubleFromObjProto TclWrapper::_Tcl_GetDoubleFromObj;
int TclWrapper::toDouble(Tcl_Interp* interpreter, Tcl_Obj* obj, double* val) {
	if (handle == nullptr || interpreter == nullptr ) { return _TCL_BOOTSTRAP_FAIL_; }
	else { return _Tcl_GetDoubleFromObj(interpreter, obj, val); }
}

template <typename T> int TclWrapper::convert(Tcl_Interp* interpreter, Tcl_Obj* obj, T* val) {
	return 0;
}
template <> int TclWrapper::convert<int>(Tcl_Interp* interpreter, Tcl_Obj* obj, int* val) {
	if (handle == nullptr || interpreter == nullptr ) { return _TCL_BOOTSTRAP_FAIL_; }
	else { return _Tcl_GetIntFromObj(interpreter, obj, val); }
}
template <> int TclWrapper::convert<long>(Tcl_Interp* interpreter, Tcl_Obj* obj, long* val) {
	if (handle == nullptr || interpreter == nullptr ) { return _TCL_BOOTSTRAP_FAIL_; }
	else { return _Tcl_GetLongFromObj(interpreter, obj, val); }
}
template <> int TclWrapper::convert<double>(Tcl_Interp* interpreter, Tcl_Obj* obj, double* val) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; }
	else { return _Tcl_GetDoubleFromObj(interpreter, obj, val); }
}

TSharedRef<TclWrapper> TclWrapper::bootstrap(uint32 _id) {
	if (handle != nullptr) { return TSharedRef<TclWrapper>(new TclWrapper(true, _id)); }
	auto dllPath = FPaths::Combine(*FPaths::GameDir(), TEXT("ThirdParty/"), TEXT(_TCL_DLL_FNAME_));
	if (FPaths::FileExists(dllPath)) {
		handle = FPlatformProcess::GetDllHandle(*dllPath);
		if (handle == nullptr) { UE_LOG(LogClass, Log, TEXT("Tcl bootstrapping failed")) }
		else {
			FString procName = "";
			procName = "Tcl_CreateInterp";
			_Tcl_CreateInterp = (_Tcl_CreateInterpProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_Eval";
			_Tcl_Eval = (_Tcl_EvalProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_CreateObjCommand";
			_Tcl_CreateObjCommand = (_Tcl_CreateObjCommandProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_ObjSetVar2";
			_Tcl_ObjSetVar2 = (_Tcl_ObjSetVar2Proto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_ObjGetVar2";
			_Tcl_ObjGetVar2 = (_Tcl_ObjGetVar2Proto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_GetObjResult";
			_Tcl_GetObjResult = (_Tcl_GetObjResultProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_SetStringObj";
			_Tcl_SetStringObj = (_Tcl_SetStringObjProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_NewStringObj";
			_Tcl_NewStringObj = (_Tcl_NewStringObjProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_NewLongObj";
			_Tcl_NewLongObj = (_Tcl_NewLongObjProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_SetIntObj";
			_Tcl_SetIntObj = (_Tcl_SetIntObjProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_GetIntFromObj";
			_Tcl_GetIntFromObj = (_Tcl_GetIntFromObjProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_GetLongFromObj";
			_Tcl_GetLongFromObj = (_Tcl_GetLongFromObjProto)FPlatformProcess::GetDllExport(handle, *procName);
			procName = "Tcl_GetDoubleFromObj";
			_Tcl_GetDoubleFromObj = (_Tcl_GetDoubleFromObjProto)FPlatformProcess::GetDllExport(handle, *procName);
			if (_Tcl_CreateInterp == nullptr ||
				_Tcl_Eval == nullptr ||
				_Tcl_CreateObjCommand == nullptr ||
				_Tcl_ObjSetVar2 == nullptr ||
				_Tcl_ObjGetVar2 == nullptr ||
				_Tcl_GetObjResult == nullptr ||
				_Tcl_SetStringObj == nullptr ||
				_Tcl_NewStringObj == nullptr ||
				_Tcl_NewLongObj == nullptr ||
				_Tcl_SetIntObj == nullptr ||
				_Tcl_GetIntFromObj == nullptr ||
				_Tcl_GetLongFromObj == nullptr ||
				_Tcl_GetDoubleFromObj == nullptr) {
				handle = nullptr;
				UE_LOG(LogClass, Log, TEXT("Bootstrapping one or more functions for Tcl failed!"))
			}
			else {
				UE_LOG(LogClass, Log, TEXT("Bootstrapping Tcl and its functions succeeded!"))
				interpreterSize = (int)sizeof(Tcl_Interp);
				return TSharedRef<TclWrapper>(new TclWrapper(true, _id));
			}
		}
	}
	else { UE_LOG(LogClass, Log, TEXT("Cannot find %s!"), _TCL_DLL_FNAME_) }
	return TSharedRef<TclWrapper>(new TclWrapper());
}
