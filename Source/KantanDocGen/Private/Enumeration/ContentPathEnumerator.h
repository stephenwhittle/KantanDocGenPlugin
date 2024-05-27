// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Copyright (C) 2016-2017 Cameron Angus. All Rights Reserved.

#pragma once

#include "ISourceObjectEnumerator.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	#include "AssetData.h"
#else
	#include "AssetRegistry/AssetData.h"
#endif


class FContentPathEnumerator: public ISourceObjectEnumerator
{
public:
	FContentPathEnumerator(
		FName const& InPath
	);


 FString GetCurrentContextString() override;

public:
	virtual UObject* GetNext() override;
	virtual float EstimateProgress() const override;
	virtual int32 EstimatedSize() const override;

protected:
	FName Path;
	void Prepass();

protected:
	TArray< FAssetData > AssetList;
	int32 CurIndex;
};


