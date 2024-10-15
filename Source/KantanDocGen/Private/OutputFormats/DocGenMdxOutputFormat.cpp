#include "OutputFormats/DocGenMdxOutputFormat.h"
#include "Json.h"
#include "Misc/FileHelper.h"
#include "OutputFormats/DocGenMdxOutputProcessor.h"
#include "OutputFormats/DocGenOutputProcessor.h"

FString DocGenMdxSerializer::EscapeString(const FString& InString)
{
	return InString;
}

FString DocGenMdxSerializer::GetFileExtension()
{
	return ".json";
}

void DocGenMdxSerializer::SerializeObject(const DocTreeNode::Object& Obj)
{
	TArray<FString> ObjectFieldNames;
	// If we have a single key...
	if (Obj.GetKeys(ObjectFieldNames) == 1)
	{
		TArray<TSharedPtr<DocTreeNode>> ArrayFieldValues;
		// If that single key has multiple values...
		Obj.MultiFind(ObjectFieldNames[0], ArrayFieldValues, true);
		if (ArrayFieldValues.Num() > 1)
		{
			// we are an array
			TargetObject = SerializeArray(ArrayFieldValues);
			return;
		}
	}

	if (!TargetObject.IsValid())
	{
		TargetObject = MakeShared<FJsonValueObject>(MakeShared<FJsonObject>());
	}
	for (auto& FieldName : ObjectFieldNames)
	{
		TArray<TSharedPtr<DocTreeNode>> ArrayFieldValues;
		Obj.MultiFind(FieldName, ArrayFieldValues, true);
		if (ArrayFieldValues.Num() > 1)
		{
			TargetObject->AsObject()->SetField(FieldName, SerializeArray(ArrayFieldValues));
		}
		else
		{
			TSharedPtr<FJsonValue> NewMemberValue;
			ArrayFieldValues[0]->SerializeWith(MakeShared<DocGenMdxSerializer>(NewMemberValue));
			TargetObject->AsObject()->SetField(FieldName, NewMemberValue);
		}
	}
}

TSharedPtr<FJsonValueArray> DocGenMdxSerializer::SerializeArray(const TArray<TSharedPtr<DocTreeNode>> ArrayElements)
{
	TArray<TSharedPtr<FJsonValue>> OutArray;
	for (auto& Element : ArrayElements)
	{
		TSharedPtr<FJsonValue> NewElementValue;
		Element->SerializeWith(MakeShared<DocGenMdxSerializer>(NewElementValue));
		OutArray.Add(NewElementValue);
	}
	return MakeShared<FJsonValueArray>(OutArray);
}

void DocGenMdxSerializer::SerializeString(const FString& InString)
{
	TargetObject = MakeShared<FJsonValueString>(InString);
}

void DocGenMdxSerializer::SerializeNull()
{
	TargetObject = MakeShared<FJsonValueNull>();
}

DocGenMdxSerializer::DocGenMdxSerializer()
	: TopLevelObject(MakeShared<FJsonValueObject>(MakeShared<FJsonObject>())),
	  TargetObject(TopLevelObject)
{}

DocGenMdxSerializer::DocGenMdxSerializer(TSharedPtr<FJsonValue>& TargetObject) : TargetObject(TargetObject) {}

bool DocGenMdxSerializer::SaveToFile(const FString& OutFileDirectory, const FString& OutFileName)
{
	if (!TopLevelObject)
	{
		return false;
	}
	else
	{
		FString Result;
		auto JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Result);
		FJsonSerializer::Serialize(TopLevelObject->AsObject().ToSharedRef(), JsonWriter);

		return FFileHelper::SaveStringToFile(Result, *(OutFileDirectory / OutFileName + GetFileExtension()),
											 FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

TSharedPtr<struct DocTreeNode::IDocTreeSerializer> UDocGenMdxOutputFactory::CreateSerializer()
{
	return MakeShared<DocGenMdxSerializer>();
}

TSharedPtr<struct IDocGenOutputProcessor> UDocGenMdxOutputFactory::CreateIntermediateDocProcessor()
{
	TOptional<FFilePath> TemplateOverride;
	if (bOverrideTemplatePath)
	{
		TemplateOverride = TemplatePath;
	}
	TOptional<FDirectoryPath> BinaryOverride;
	if (bOverrideBinaryPath)
	{
		BinaryOverride = BinaryPath;
	}
	TOptional<FFilePath> NpmOverride;
	if (bOverrideNpmPath)
	{
		NpmOverride = NpmPath;
	}
	TOptional<FDirectoryPath> DocRootOverride;
	if (bOverrideDocRootPath)
	{
		DocRootOverride = DocRootPath;
	}
	TOptional<FDirectoryPath> DocusaurusOverride;
	if (bOverrideDocusaurusPath)
	{
		DocusaurusOverride = DocusaurusPath;
	}
	return MakeShared<DocGenMdxOutputProcessor>(TemplateOverride, BinaryOverride, NpmOverride, DocRootOverride,
												DocusaurusOverride);
}

FString UDocGenMdxOutputFactory::GetFormatIdentifier()
{
	return "json";
}

void UDocGenMdxOutputFactory::LoadSettings(const FDocGenOutputFormatFactorySettings& Settings)
{
	if (Settings.SettingValues.Contains("template"))
	{
		TemplatePath.FilePath = Settings.SettingValues["template"];
		if (Settings.SettingValues.Contains("overridetemplate"))
		{
			bOverrideTemplatePath = (Settings.SettingValues["overridetemplate"] == "true");
		}
	}
	if (Settings.SettingValues.Contains("bindir"))
	{
		BinaryPath.Path = Settings.SettingValues["bindir"];
		if (Settings.SettingValues.Contains("overridebindir"))
		{
			bOverrideBinaryPath = (Settings.SettingValues["overridebindir"] == "true");
		}
	}
	if (Settings.SettingValues.Contains("npm"))
	{
		NpmPath.FilePath = Settings.SettingValues["npm"];
		if (Settings.SettingValues.Contains("overridenpm"))
		{
			bOverrideNpmPath = (Settings.SettingValues["overridenpm"] == "true");
		}
	}
	if (Settings.SettingValues.Contains("docroot"))
	{
		DocRootPath.Path = Settings.SettingValues["docroot"];
		if (Settings.SettingValues.Contains("overridedocroot"))
		{
			bOverrideDocRootPath = (Settings.SettingValues["overridedocroot"] == "true");
		}
	}
	if (Settings.SettingValues.Contains("docusaurus"))
	{
		DocusaurusPath.Path = Settings.SettingValues["docusaurus"];
		if (Settings.SettingValues.Contains("overridedocusaurus"))
		{
			bOverrideDocusaurusPath = (Settings.SettingValues["overridedocusaurus"] == "true");
		}
	}
}

FDocGenOutputFormatFactorySettings UDocGenMdxOutputFactory::SaveSettings()
{
	FDocGenOutputFormatFactorySettings Settings;
	if (bOverrideTemplatePath)
	{
		Settings.SettingValues.Add("overridetemplate", "true");
	}
	Settings.SettingValues.Add("template", TemplatePath.FilePath);

	if (bOverrideBinaryPath)
	{
		Settings.SettingValues.Add("overridebindir", "true");
	}
	Settings.SettingValues.Add("bindir", BinaryPath.Path);

	if (bOverrideNpmPath)
	{
		Settings.SettingValues.Add("overridenpm", "true");
	}
	Settings.SettingValues.Add("npm", NpmPath.FilePath);

	if (bOverrideDocRootPath)
	{
		Settings.SettingValues.Add("overridedocroot", "true");
	}
	Settings.SettingValues.Add("docroot", DocRootPath.Path);

	if (bOverrideDocusaurusPath)
	{
		Settings.SettingValues.Add("overridedocusaurus", "true");
	}
	Settings.SettingValues.Add("docusaurus", DocusaurusPath.Path);

	Settings.FactoryClass = StaticClass();
	return Settings;
}
