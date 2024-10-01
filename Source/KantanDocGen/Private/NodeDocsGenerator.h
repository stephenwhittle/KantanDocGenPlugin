// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Copyright (C) 2016-2017 Cameron Angus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"
#include "Slate/WidgetRenderer.h"

class UClass;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;
class UK2Node;
class UBlueprintNodeSpawner;
class FXmlFile;

class FNodeDocsGenerator
{
public:
	FNodeDocsGenerator(const TArray<class UDocGenOutputFormatFactoryBase*>& OutputFormats)
		: Renderer(false),
		  OutputFormats(OutputFormats)
	{}
	~FNodeDocsGenerator();

public:
	struct FNodeProcessingState
	{
		TSharedPtr<class DocTreeNode> ClassDocTree;
		FString ClassDocsPath;
		FString RelImageBasePath;
		FString ImageFilename;
		FString NodeClassId;
		FNodeProcessingState() : ClassDocTree(), ClassDocsPath(), RelImageBasePath(), ImageFilename(), NodeClassId() {}
	};

public:
	/** Callable only from game thread */
	bool GT_Init(FString const& InDocsTitle, FString const& InOutputDir,
				 UClass* BlueprintContextClass = AActor::StaticClass());
	UK2Node* GT_InitializeForSpawner(UBlueprintNodeSpawner* Spawner, UObject* SourceObject,
									 FNodeProcessingState& OutState);
	bool GT_Finalize(FString OutputPath);
	/**/

	/** Callable from background thread */
	bool GenerateNodeImage(UEdGraphNode* Node, FNodeProcessingState& State);
	bool GenerateNodeDocTree(UK2Node* Node, FNodeProcessingState& State);

	bool GenerateWidgetImage(UObject* ClassObject);

	bool GenerateTypeMembers(UObject* Type);
	/**/

protected:
	void CleanUp();
	bool SaveIndexFile(FString const& OutDir);
	bool SaveClassDocFile(FString const& OutDir);
	bool SaveEnumDocFile(FString const& OutDir);
	bool SaveStructDocFile(FString const& OutDir);
	bool SaveDelegateDocFile(FString const& OutDir);

	TSharedPtr<DocTreeNode> InitIndexDocTree(FString const& IndexTitle);
	TSharedPtr<DocTreeNode> InitClassDocTree(UClass* Class);
	TSharedPtr<DocTreeNode> InitStructDocTree(UScriptStruct* Struct);
	TSharedPtr<DocTreeNode> InitEnumDocTree(UEnum* Enum);
	TSharedPtr<DocTreeNode> InitDelegateDocTree(UFunction* SignatureFunction);

	void AddMetaDataMapToNode(TSharedPtr<DocTreeNode> Node, const TMap<FName, FString>* MetaDataMap);
	FString GenerateFunctionSignatureString(UFunction* Func, bool bUseFuncPtrStyle = false);
	bool UpdateIndexDocWithClass(TSharedPtr<DocTreeNode> DocTree, UClass* Class);
	bool UpdateIndexDocWithStruct(TSharedPtr<DocTreeNode> DocTree, UStruct* Struct);
	bool UpdateIndexDocWithEnum(TSharedPtr<DocTreeNode> DocTree, UEnum* Enum);
	bool UpdateIndexDocWithDelegate(TSharedPtr<DocTreeNode> DocTree, UFunction* SignatureFunction);
	bool UpdateClassDocWithNode(TSharedPtr<DocTreeNode> DocTree, UEdGraphNode* Node);

	static void AdjustNodeForSnapshot(UEdGraphNode* Node);
	static FString GetClassDocId(UClass* Class);
	static FString GetNodeDocId(UEdGraphNode* Node);
	FString GetDelegateDocId(UFunction* SignatureFunction, bool bStripMulticast = true);
	static UClass* MapToAssociatedClass(UK2Node* NodeInst, UObject* Source);
	static bool IsSpawnerDocumentable(UBlueprintNodeSpawner* Spawner, bool bIsBlueprint);

protected:
	TWeakObjectPtr<UBlueprint> DummyBP;
	TWeakObjectPtr<UEdGraph> Graph;
	TSharedPtr<class SGraphPanel> GraphPanel;
	FWidgetRenderer Renderer;

	FString DocsTitle;
	TSharedPtr<DocTreeNode> IndexTree;
	TMap<TWeakObjectPtr<UClass>, TSharedPtr<DocTreeNode>> ClassDocTreeMap;
	TMap<TWeakObjectPtr<UStruct>, TSharedPtr<DocTreeNode>> StructDocTreeMap;
	TMap<TWeakObjectPtr<UEnum>, TSharedPtr<DocTreeNode>> EnumDocTreeMap;
	TMap<FString, TSharedPtr<DocTreeNode>> DelegateDocTreeMap;
	TArray<UDocGenOutputFormatFactoryBase*> OutputFormats;
	FString OutputDir;
	bool SaveAllFormats(FString const& OutDir, TSharedPtr<DocTreeNode> Document)
	{
		return false;
	};

public:
	//
	double GenerateNodeImageTime = 0.0;
	double GenerateNodeDocsTime = 0.0;
	//
	FString ContextString;
};
