// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Copyright (C) 2016-2017 Cameron Angus. All Rights Reserved.

#include "ContentPathEnumerator.h"
#include "KantanDocGenLog.h"
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	#include "AssetRegistryModule.h"
	#include "ARFilter.h"
#else
	#include "AssetRegistry/AssetRegistryModule.h"
	#include "AssetRegistry/ARFilter.h"
#endif
#include "Engine/Blueprint.h"
#include "Animation/AnimBlueprint.h"


FContentPathEnumerator::FContentPathEnumerator(
	FName const& InPath
)
{
	CurIndex = 0;
	Path = InPath;
	Prepass();
}


FString FContentPathEnumerator::GetCurrentContextString()
{
	return Path.ToString();
}

void FContentPathEnumerator::Prepass()
{
	auto& AssetRegistryModule = FModuleManager::GetModuleChecked< FAssetRegistryModule >("AssetRegistry");
	auto& AssetRegistry = AssetRegistryModule.Get();

	// Get all the asset in our content folders
	AssetRegistry.SearchAllAssets(true);
	while (AssetRegistry.IsLoadingAssets())
	{
		AssetRegistry.Tick(1.0f);
	}

	FARFilter Filter;
	Filter.bRecursiveClasses = true;
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	#else
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	#endif

	#if UE_VERSION_OLDER_THAN(5, 3, 0)
	// @TODO: Not sure about this, but for some reason was generating docs for 'AnimInstance' itself.
	Filter.RecursiveClassesExclusionSet.Add(UAnimBlueprint::StaticClass()->GetFName());
	#else
	Filter.RecursiveClassPathsExclusionSet.Add(UAnimBlueprint::StaticClass()->GetClassPathName());
	#endif

	AssetRegistry.GetAssetsByPath(Path, AssetList, true);
	UE_LOG(LogKantanDocGen, Log, TEXT("Found %d assets at '%s'"), AssetList.Num(), *Path.ToString());		
	AssetRegistry.RunAssetsThroughFilter(AssetList, Filter);
	UE_LOG(LogKantanDocGen, Log, TEXT("%d assets passed filtering"), AssetList.Num());
}

UObject* FContentPathEnumerator::GetNext()
{
	UObject* Result = nullptr;

	while(CurIndex < AssetList.Num())
	{
		auto const& AssetData = AssetList[CurIndex];
		++CurIndex;

		if(auto Blueprint = Cast< UBlueprint >(AssetData.GetAsset()))
		{
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			UE_LOG(LogKantanDocGen, Log, TEXT("Enumerating object '%s' at '%s'"), *Blueprint->GetName(), *AssetData.ObjectPath.ToString());
#else
			UE_LOG(LogKantanDocGen, Log, TEXT("Enumerating object '%s' at '%s'"), *Blueprint->GetName(), *AssetData.GetSoftObjectPath().ToString());
			#endif
			Result = Blueprint;
			break;
		}
	}
	
	return Result;
}

float FContentPathEnumerator::EstimateProgress() const
{
	return (float)CurIndex / (AssetList.Num() - 1);
}

int32 FContentPathEnumerator::EstimatedSize() const
{
	return AssetList.Num();
}

