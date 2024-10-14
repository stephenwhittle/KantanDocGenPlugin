#pragma once

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "DocTreeNode.h"
#include "OutputFormats/DocGenOutputFormatFactoryBase.h"
#include "Templates/SharedPointer.h"

#include "DocGenMdxOutputFormat.generated.h"

class DocGenMdxSerializer : public DocTreeNode::IDocTreeSerializer
{
	TSharedPtr<class FJsonValue> TopLevelObject;
	TSharedPtr<FJsonValue>& TargetObject;

	virtual FString EscapeString(const FString& InString) override;
	virtual FString GetFileExtension() override;
	virtual void SerializeObject(const DocTreeNode::Object& Obj) override;

	TSharedPtr<class FJsonValueArray> SerializeArray(const TArray<TSharedPtr<DocTreeNode>> ArrayElements);

	virtual void SerializeString(const FString& InString) override;
	virtual void SerializeNull() override;

public:
	DocGenMdxSerializer(TSharedPtr<FJsonValue>& TargetObject);;
	DocGenMdxSerializer();
	virtual bool SaveToFile(const FString& OutFileDirectory, const FString& OutFileName);;
};

UCLASS(meta = (DisplayName = "MDX"), Meta = (ShowOnlyInnerProperties), Config = EditorPerProjectUserSettings)
class UDocGenMdxOutputFactory : public UDocGenOutputFormatFactoryBase
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<struct DocTreeNode::IDocTreeSerializer> CreateSerializer() override;
	virtual TSharedPtr<struct IDocGenOutputProcessor> CreateIntermediateDocProcessor() override;
	virtual FString GetFormatIdentifier() override;

	virtual void LoadSettings(const FDocGenOutputFormatFactorySettings& Settings);

	virtual FDocGenOutputFormatFactorySettings SaveSettings();;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bOverrideTemplatePath = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bOverrideTemplatePath"))
	FFilePath TemplatePath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bOverrideBinaryPath = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bOverrideBinaryPath"))
	FDirectoryPath BinaryPath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bOverrideNpmPath = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bOverrideNpmPath"))
	FFilePath NpmPath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bOverrideDocRootPath = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bOverrideDocRootPath"))
	FDirectoryPath DocRootPath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bOverrideDocusaurusPath = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bOverrideDocusaurusPath"))
	FDirectoryPath DocusaurusPath;
};