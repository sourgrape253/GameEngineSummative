// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "ObjectMacros.h"
#include "ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef CHAT_chatGameModeBase_generated_h
#error "chatGameModeBase.generated.h already included, missing '#pragma once' in chatGameModeBase.h"
#endif
#define CHAT_chatGameModeBase_generated_h

#define chat_Source_chat_chatGameModeBase_h_15_RPC_WRAPPERS
#define chat_Source_chat_chatGameModeBase_h_15_RPC_WRAPPERS_NO_PURE_DECLS
#define chat_Source_chat_chatGameModeBase_h_15_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAchatGameModeBase(); \
	friend CHAT_API class UClass* Z_Construct_UClass_AchatGameModeBase(); \
public: \
	DECLARE_CLASS(AchatGameModeBase, AGameModeBase, COMPILED_IN_FLAGS(0 | CLASS_Transient), 0, TEXT("/Script/chat"), NO_API) \
	DECLARE_SERIALIZER(AchatGameModeBase) \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC};


#define chat_Source_chat_chatGameModeBase_h_15_INCLASS \
private: \
	static void StaticRegisterNativesAchatGameModeBase(); \
	friend CHAT_API class UClass* Z_Construct_UClass_AchatGameModeBase(); \
public: \
	DECLARE_CLASS(AchatGameModeBase, AGameModeBase, COMPILED_IN_FLAGS(0 | CLASS_Transient), 0, TEXT("/Script/chat"), NO_API) \
	DECLARE_SERIALIZER(AchatGameModeBase) \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC};


#define chat_Source_chat_chatGameModeBase_h_15_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API AchatGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(AchatGameModeBase) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AchatGameModeBase); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AchatGameModeBase); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API AchatGameModeBase(AchatGameModeBase&&); \
	NO_API AchatGameModeBase(const AchatGameModeBase&); \
public:


#define chat_Source_chat_chatGameModeBase_h_15_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API AchatGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API AchatGameModeBase(AchatGameModeBase&&); \
	NO_API AchatGameModeBase(const AchatGameModeBase&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AchatGameModeBase); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AchatGameModeBase); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(AchatGameModeBase)


#define chat_Source_chat_chatGameModeBase_h_15_PRIVATE_PROPERTY_OFFSET
#define chat_Source_chat_chatGameModeBase_h_12_PROLOG
#define chat_Source_chat_chatGameModeBase_h_15_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	chat_Source_chat_chatGameModeBase_h_15_PRIVATE_PROPERTY_OFFSET \
	chat_Source_chat_chatGameModeBase_h_15_RPC_WRAPPERS \
	chat_Source_chat_chatGameModeBase_h_15_INCLASS \
	chat_Source_chat_chatGameModeBase_h_15_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define chat_Source_chat_chatGameModeBase_h_15_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	chat_Source_chat_chatGameModeBase_h_15_PRIVATE_PROPERTY_OFFSET \
	chat_Source_chat_chatGameModeBase_h_15_RPC_WRAPPERS_NO_PURE_DECLS \
	chat_Source_chat_chatGameModeBase_h_15_INCLASS_NO_PURE_DECLS \
	chat_Source_chat_chatGameModeBase_h_15_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID chat_Source_chat_chatGameModeBase_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
