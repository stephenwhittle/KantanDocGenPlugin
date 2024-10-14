#include "OutputFormats/DocGenMdxOutputProcessor.h"
#include "Algo/Transform.h"
#include "HAL/FileManager.h"

// To define the UE_5_0_OR_LATER below
#include "Misc/EngineVersionComparison.h"
#if UE_VERSION_NEWER_THAN(5, 0, 0)
	#include "HAL/PlatformFileManager.h"
#else
	#include "HAL/PlatformFilemanager.h"
#endif

#include "Json.h"
#include "JsonDomBuilder.h"
#include "KantanDocGenLog.h"
#include "Misc/FileHelper.h"
#include "Misc/Optional.h"
#include "Misc/Paths.h"

FString DocGenMdxOutputProcessor::Quote(const FString& In)
{
	if (In.TrimStartAndEnd().StartsWith("\""))
	{
		return In;
	}
	return "\"" + In.TrimStartAndEnd() + "\"";
}

TOptional<FString> DocGenMdxOutputProcessor::GetObjectStringField(const TSharedPtr<FJsonObject> Obj,
																  const FString& FieldName)
{
	FString FieldValue;
	if (!Obj->TryGetStringField(FieldName, FieldValue))
	{
		return {};
	}
	else
	{
		return FieldValue;
	}
}

TOptional<FString> DocGenMdxOutputProcessor::GetObjectStringField(const TSharedPtr<FJsonValue> Obj,
																  const FString& FieldName)
{
	const TSharedPtr<FJsonObject>* UnderlyingObject = nullptr;
	if (!Obj->TryGetObject(UnderlyingObject))
	{
		return {};
	}
	else
	{
		return GetObjectStringField(*UnderlyingObject, FieldName);
	}
}

TOptional<TArray<FString>> DocGenMdxOutputProcessor::GetNamesFromFileAtLocation(const FString& NameType,
																				const FString& ClassFile)
{
	TSharedPtr<FJsonObject> ParsedClass = LoadFileToJson(ClassFile);
	if (!ParsedClass)
	{
		return {};
	}

	if (ParsedClass->HasTypedField<EJson::Array>(NameType))
	{
		TArray<FString> NodeNames;
		for (const auto& Value : ParsedClass->GetArrayField(NameType))
		{
			TOptional<FString> FuncID = GetObjectStringField(Value, "id");
			if (FuncID.IsSet())
			{
				NodeNames.Add(FuncID.GetValue());
			}
		}
		return NodeNames;
	}
	else if (ParsedClass->HasTypedField<EJson::Object>(NameType))
	{
		TArray<FString> NodeNames;
		for (const auto& Node : ParsedClass->GetObjectField(NameType)->Values)
		{
			TOptional<FString> Name = GetObjectStringField(Node.Value, "id");
			if (Name.IsSet())
			{
				NodeNames.Add(Name.GetValue());
			}
		}
		return NodeNames;
	}
	else if (ParsedClass->HasTypedField<EJson::Null>(NameType))
	{
		return TArray<FString>();
	}
	return {};
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseNodeFile(const FString& NodeFilePath)
{
	TSharedPtr<FJsonObject> ParsedNode = LoadFileToJson(NodeFilePath);
	if (!ParsedNode)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();

	CopyJsonField("inputs", ParsedNode, OutNode);
	CopyJsonField("outputs", ParsedNode, OutNode);
	CopyJsonField("rawsignature", ParsedNode, OutNode);
	CopyJsonField("class_id", ParsedNode, OutNode);
	CopyJsonField("doxygen", ParsedNode, OutNode);
	CopyJsonField("imgpath", ParsedNode, OutNode);
	CopyJsonField("shorttitle", ParsedNode, OutNode);
	CopyJsonField("fulltitle", ParsedNode, OutNode);
	CopyJsonField("static", ParsedNode, OutNode);
	CopyJsonField("autocast", ParsedNode, OutNode);
	CopyJsonField("funcname", ParsedNode, OutNode);
	CopyJsonField("access_specifier", ParsedNode, OutNode);
	CopyJsonField("meta", ParsedNode, OutNode);
	return OutNode;
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseClassFile(const FString& ClassFilePath)
{
	TSharedPtr<FJsonObject> ParsedClass = LoadFileToJson(ClassFilePath);
	if (!ParsedClass)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();
	// Reusing the class template for now so renaming id to class_id to be consistent
	if (TSharedPtr<FJsonValue> Field = ParsedClass->TryGetField("id"))
	{
		OutNode->SetField("class_id", Field);
	}

	CopyJsonField("doxygen", ParsedClass, OutNode);
	CopyJsonField("display_name", ParsedClass, OutNode);
	CopyJsonField("fields", ParsedClass, OutNode);
	CopyJsonField("parent_class", ParsedClass, OutNode);
	CopyJsonField("meta", ParsedClass, OutNode);
	CopyJsonField("blueprint_generated", ParsedClass, OutNode);
	CopyJsonField("widget_blueprint", ParsedClass, OutNode);
	CopyJsonField("class_path", ParsedClass, OutNode);
	CopyJsonField("context_string", ParsedClass, OutNode);
	return OutNode;
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseStructFile(const FString& StructFilePath)
{
	TSharedPtr<FJsonObject> ParsedStruct = LoadFileToJson(StructFilePath);
	if (!ParsedStruct)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();
	// Reusing the class template for now so renaming id to class_id to be consistent
	if (TSharedPtr<FJsonValue> Field = ParsedStruct->TryGetField("id"))
	{
		OutNode->SetField("class_id", Field);
	}

	CopyJsonField("doxygen", ParsedStruct, OutNode);
	CopyJsonField("display_name", ParsedStruct, OutNode);
	CopyJsonField("fields", ParsedStruct, OutNode);
	CopyJsonField("parent_class", ParsedStruct, OutNode);
	CopyJsonField("meta", ParsedStruct, OutNode);
	CopyJsonField("blueprint_generated", ParsedStruct, OutNode);
	CopyJsonField("widget_blueprint", ParsedStruct, OutNode);
	CopyJsonField("class_path", ParsedStruct, OutNode);
	CopyJsonField("context_string", ParsedStruct, OutNode);
	return OutNode;
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseEnumFile(const FString& EnumFilePath)
{
	TSharedPtr<FJsonObject> ParsedEnum = LoadFileToJson(EnumFilePath);
	if (!ParsedEnum)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();

	CopyJsonField("id", ParsedEnum, OutNode);
	CopyJsonField("doxygen", ParsedEnum, OutNode);
	CopyJsonField("display_name", ParsedEnum, OutNode);
	CopyJsonField("values", ParsedEnum, OutNode);
	CopyJsonField("meta", ParsedEnum, OutNode);

	return OutNode;
}

void DocGenMdxOutputProcessor::CopyJsonField(const FString& FieldName, TSharedPtr<FJsonObject> ParsedNode,
											 TSharedPtr<FJsonObject> OutNode)
{
	if (TSharedPtr<FJsonValue> Field = ParsedNode->TryGetField(FieldName))
	{
		OutNode->SetField(FieldName, Field);
	}
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::InitializeMainOutputFromIndex(TSharedPtr<FJsonObject> ParsedIndex)
{
	TSharedPtr<FJsonObject> Output = MakeShared<FJsonObject>();

	CopyJsonField("display_name", ParsedIndex, Output);
	return Output;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConvertJsonToMdx(FString IntermediateDir)
{
	const FFilePath InJsonPath {IntermediateDir / "consolidated.json"};
	const FFilePath OutMdxPath {IntermediateDir / "docs.mdx"};
	const FString Format {"markdown"};

	void* PipeRead = nullptr;
	void* PipeWrite = nullptr;
	verify(FPlatformProcess::CreatePipe(PipeRead, PipeWrite));

	const FString Args = Quote(TemplatePath.FilePath) + " " + Quote(InJsonPath.FilePath) + " " +
						 Quote(OutMdxPath.FilePath) + " " + Quote(Format);

	FProcHandle Proc = FPlatformProcess::CreateProc(*(BinaryPath.Path / "convert.exe"), *Args, true, false, false,
													nullptr, 0, nullptr, PipeWrite);

	int32 ReturnCode = 0;
	if (Proc.IsValid())
	{
		FString BufferedText;
		for (bool bProcessFinished = false; !bProcessFinished;)
		{
			bProcessFinished = FPlatformProcess::GetProcReturnCode(Proc, &ReturnCode);

			/*
			if(!bProcessFinished && Warn->ReceivedUserCancel())
			{
			FPlatformProcess::TerminateProc(ProcessHandle);
			bProcessFinished = true;
			}
			*/
			BufferedText += FPlatformProcess::ReadPipe(PipeRead);
			int32 EndOfLineIdx;
			while (BufferedText.FindChar('\n', EndOfLineIdx))
			{
				FString Line = BufferedText.Left(EndOfLineIdx);
				Line.RemoveFromEnd(TEXT("\r"));

				UE_LOG(LogKantanDocGen, Error, TEXT("[KantanDocGen] %s"), *Line);

				BufferedText = BufferedText.Mid(EndOfLineIdx + 1);
			}

			FPlatformProcess::Sleep(0.1f);
		}
		FPlatformProcess::CloseProc(Proc);
		Proc.Reset();

		if (ReturnCode != 0)
		{
			UE_LOG(LogKantanDocGen, Error, TEXT("KantanDocGen tool failed (code %i), see above output."), ReturnCode);
			return EIntermediateProcessingResult::UnknownError;
		}

		// After successfully generating the .mdx docs and image files, copy them to doc_root
		IFileManager& FileManager = IFileManager::Get();
		FFilePath MdxDestinationPath {DocRootPath.Path / "en-us/generated-refdocs.mdx"};
		FDirectoryPath ImgDestinationPath {DocRootPath.Path / "en-us/img/generated-refdocs"};

		if (FileManager.Copy(*MdxDestinationPath.FilePath, *OutMdxPath.FilePath) != 0)
		{
			UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy %s to %s"), *OutMdxPath.FilePath,
				   *MdxDestinationPath.FilePath);
			return EIntermediateProcessingResult::UnknownError;
		}

		FileManager.DeleteDirectory(*ImgDestinationPath.Path);
		TArray<FString> ImgDirectories;
		FileManager.FindFilesRecursive(ImgDirectories, *IntermediateDir, TEXT("img"), false, true);
		for (FString ImgDirectory : ImgDirectories)
		{
			TArray<FString> ImageFiles;
			FileManager.FindFiles(ImageFiles, *ImgDirectory, TEXT("png"));
			for (FString Image : ImageFiles)
			{
				FString SourceImagePath {ImgDirectory / Image};
				FString DestinationImagePath {ImgDestinationPath.Path / Image};
				if (FileManager.Copy(*DestinationImagePath, *SourceImagePath) != 0)
				{
					UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy %s to %s"), *SourceImagePath,
						   *DestinationImagePath);
					return EIntermediateProcessingResult::UnknownError;
				}
			}
		}
		return EIntermediateProcessingResult::Success;
	}
	else
	{
		return EIntermediateProcessingResult::UnknownError;
	}
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConvertMdxToHtml(FString IntermediateDir, FString OutputDir)
{
	// create a docusaurus staging directory
	const FString DocusaurusStagingPath {IntermediateDir / "docusaurus"};
	if (!FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*DocusaurusStagingPath, *DocusaurusPath.Path,
																		 true))
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy template docusaurus %s to intermediate directory %s"),
			   *DocusaurusPath.Path, *DocusaurusStagingPath);
		return EIntermediateProcessingResult::UnknownError;
	}
	// merge doc_root mdx and img into staging directory
	if (!FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*(DocusaurusStagingPath / "public/en-us"),
																		 *(DocRootPath.Path / "en-us"), false))
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to merge doc_root %s into docusaurus staging directory %s"),
			   *(DocRootPath.Path / "en-us"), *(DocusaurusStagingPath / "public/en-us"));
		return EIntermediateProcessingResult::UnknownError;
	}
	// copy sidebars.js, replacing the existing file
	if (!FPlatformFileManager::Get().GetPlatformFile().CopyFile(*(DocusaurusStagingPath / "public/menu/sidebars.js"),
																*(DocRootPath.Path / "menu/sidebars.js")))
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to merge doc_root %s into staging directory %s"),
			   *(DocRootPath.Path / "menu/sidebars.js"), *(DocusaurusStagingPath / "public/menu/sidebars.js"));
		return EIntermediateProcessingResult::UnknownError;
	}

	// invoke npm install to install required packages
	FProcHandle InstallProcessHandle =
		FPlatformProcess::CreateProc(*(NpmExecutablePath.FilePath), TEXT("install"), true, false, false, nullptr, 0,
									 *DocusaurusStagingPath, nullptr, nullptr, nullptr);
	if (!InstallProcessHandle.IsValid())
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Create process for npm install step failed"));
		return EIntermediateProcessingResult::UnknownError;
	}
	FPlatformProcess::WaitForProc(InstallProcessHandle);
	int32 InstallExitCode;
	FPlatformProcess::GetProcReturnCode(InstallProcessHandle, &InstallExitCode);
	FPlatformProcess::CloseProc(InstallProcessHandle);
	if (InstallExitCode != 0)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("npm install error"));
		return EIntermediateProcessingResult::UnknownError;
	}
	// invoke npm run build to build the html docs
	FProcHandle BuildProcessHandle =
		FPlatformProcess::CreateProc(*(NpmExecutablePath.FilePath), TEXT("run build"), true, false, false, nullptr, 0,
									 *DocusaurusStagingPath, nullptr, nullptr, nullptr);
	if (!BuildProcessHandle.IsValid())
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Create process for npm build step failed"));
		return EIntermediateProcessingResult::UnknownError;
	}
	FPlatformProcess::WaitForProc(BuildProcessHandle);
	int32 BuildExitCode;
	FPlatformProcess::GetProcReturnCode(BuildProcessHandle, &BuildExitCode);
	FPlatformProcess::CloseProc(BuildProcessHandle);
	if (BuildExitCode != 0)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("npm run build error"));
		return EIntermediateProcessingResult::UnknownError;
	}
	// copy result from intermediate directory to output directory
	FString HtmlDocsPath {DocusaurusStagingPath / "build"};
	//FString HtmlOutputDir {"C:/dev/DocsTest"};
	if (!FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*OutputDir, *HtmlDocsPath, true))
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy build docs %s to output directory %s"), *HtmlDocsPath,
			   *OutputDir);
		return EIntermediateProcessingResult::UnknownError;
	}
	return EIntermediateProcessingResult::Success;
}

DocGenMdxOutputProcessor::DocGenMdxOutputProcessor(TOptional<FFilePath> TemplatePathOverride,
												   TOptional<FDirectoryPath> BinaryPathOverride,
												   TOptional<FFilePath> NpmPathOverride,
												   TOptional<FDirectoryPath> DocRootPathOverride,
												   TOptional<FDirectoryPath> DocusaurusPathOverride)
{
	if (BinaryPathOverride.IsSet())
	{
		BinaryPath = BinaryPathOverride.GetValue();
	}
	else
	{
		BinaryPath.Path = FString("bin");
	}
	if (TemplatePathOverride.IsSet())
	{
		TemplatePath = TemplatePathOverride.GetValue();
	}
	else
	{
		TemplatePath.FilePath = BinaryPath.Path / "template" / "docs.mdx.in";
	}
	if (NpmPathOverride.IsSet())
	{
		NpmExecutablePath = NpmPathOverride.GetValue();
	}
	else
	{
		NpmExecutablePath.FilePath = "C:/Program Files/nodejs/npm.cmd";
	}
	if (DocRootPathOverride.IsSet())
	{
		DocRootPath = DocRootPathOverride.GetValue();
	}
	else
	{
		DocRootPath.Path = BinaryPath.Path / "doc_root";
	}
	if (DocusaurusPathOverride.IsSet())
	{
		DocusaurusPath = DocusaurusPathOverride.GetValue();
	}
	else
	{
		DocusaurusPath.Path = FPaths::ProjectDir() / "Doc/docusaurus";
	}
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ProcessIntermediateDocs(FString const& IntermediateDir,
																				FString const& OutputDir,
																				FString const& DocTitle,
																				bool bCleanOutput)
{
	TSharedPtr<FJsonObject> ParsedIndex = LoadFileToJson(IntermediateDir / "index.json");

	TSharedPtr<FJsonObject> ConsolidatedOutput = InitializeMainOutputFromIndex(ParsedIndex);

	EIntermediateProcessingResult ClassResult =
		ConsolidateClasses(ParsedIndex, IntermediateDir, OutputDir, ConsolidatedOutput);
	if (ClassResult != EIntermediateProcessingResult::Success)
	{
		return ClassResult;
	}

	EIntermediateProcessingResult StructResult =
		ConsolidateStructs(ParsedIndex, IntermediateDir, OutputDir, ConsolidatedOutput);
	if (StructResult != EIntermediateProcessingResult::Success)
	{
		return StructResult;
	}

	EIntermediateProcessingResult EnumResult =
		ConsolidateEnums(ParsedIndex, IntermediateDir, OutputDir, ConsolidatedOutput);
	if (EnumResult != EIntermediateProcessingResult::Success)
	{
		return EnumResult;
	}

	FString Result;
	auto JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Result);
	FJsonSerializer::Serialize(ConsolidatedOutput.ToSharedRef(), JsonWriter);

	if (!FFileHelper::SaveStringToFile(Result, *(IntermediateDir / "consolidated.json"),
									   FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		return EIntermediateProcessingResult::DiskWriteFailure;
	}
	// create mdx and image files, and copy them to doc_root
	if (ConvertJsonToMdx(IntermediateDir) == EIntermediateProcessingResult::Success)
	{
		// invoke npm to convert mdx to html, and copy results to specified output directory
		return ConvertMdxToHtml(IntermediateDir, OutputDir);
	}
	return EIntermediateProcessingResult::UnknownError;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConsolidateClasses(TSharedPtr<FJsonObject> ParsedIndex,
																		   FString const& IntermediateDir,
																		   FString const& OutputDir,
																		   TSharedPtr<FJsonObject> ConsolidatedOutput)
{
	FJsonDomBuilder::FArray StaticFunctionList;
	FJsonDomBuilder::FObject ClassFunctionList;
	TOptional<TArray<FString>> ClassNames = GetNamesFromIndexFile("classes", ParsedIndex);
	if (!ClassNames.IsSet())
	{
		return EIntermediateProcessingResult::UnknownError;
	}

	for (const auto& ClassName : ClassNames.GetValue())
	{
		const FString ClassFilePath = IntermediateDir / ClassName / ClassName + ".json";
		TOptional<TArray<FString>> NodeNames = GetNamesFromFileAtLocation("nodes", ClassFilePath);
		TSharedPtr<FJsonObject> ParsedClass = ParseStructFile(ClassFilePath);
		if (!NodeNames.IsSet())
		{
			return EIntermediateProcessingResult::UnknownError;
		}
		else
		{
			TOptional<FString> NodeClassID;
			FJsonDomBuilder::FArray Nodes;
			for (const auto& NodeName : NodeNames.GetValue())
			{
				const FString NodeFilePath = IntermediateDir / ClassName / "nodes" / NodeName + ".json";

				if (TSharedPtr<FJsonObject> NodeJson = ParseNodeFile(NodeFilePath))
				{
					FString RelImagePath;
					if (NodeJson->TryGetStringField("imgpath", RelImagePath))
					{
						FString SourceImagePath = IntermediateDir / ClassName / "nodes" / RelImagePath;
						SourceImagePath =
							IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*SourceImagePath);
						IFileManager::Get().Copy(*(OutputDir / "img" / FPaths::GetCleanFilename(RelImagePath)),
												 *SourceImagePath, true);
					}
					bool FunctionIsStatic = false;
					NodeJson->TryGetBoolField("static", FunctionIsStatic);

					if (FunctionIsStatic)
					{
						StaticFunctionList.Add(MakeShared<FJsonValueObject>(NodeJson));
					}
					else
					{
						Nodes.Add(MakeShared<FJsonValueObject>(NodeJson));
					}
				}
				else
				{
					return EIntermediateProcessingResult::UnknownError;
				}
			}
			// We don't want classes in our classlist if all their nodes are static
			FJsonDomBuilder::FObject ClassObj;
			ClassObj.Set("functions", Nodes);
			ClassObj.Set("class_id", ClassName);
			ClassObj.Set("display_name", ParsedClass->GetStringField("display_name"));
			ClassObj.Set("meta", MakeShared<FJsonValueObject>(ParsedClass->GetObjectField("meta")));
			ClassObj.Set("parent_class", MakeShared<FJsonValueObject>(ParsedClass->GetObjectField("parent_class")));
			const TSharedPtr<FJsonObject>* DoxygenBlock;
			bool bHadDoxygenBlock = ParsedClass->TryGetObjectField("doxygen", DoxygenBlock);
			if (bHadDoxygenBlock)
			{
				ClassObj.Set("doxygen", MakeShared<FJsonValueObject>(*DoxygenBlock));
			}
			const TArray<TSharedPtr<FJsonValue>>* FieldArray;
			bool bHadFields = ParsedClass->TryGetArrayField("fields", FieldArray);
			ClassObj.Set("fields", bHadFields ? MakeShared<FJsonValueArray>(*FieldArray)
											  : MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>> {}));

			ClassFunctionList.Set(ClassName, ClassObj);
		}
	}

	ConsolidatedOutput->SetField("functions", StaticFunctionList.AsJsonValue());
	ConsolidatedOutput->SetField("classes", ClassFunctionList.AsJsonValue());
	return EIntermediateProcessingResult::Success;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConsolidateStructs(TSharedPtr<FJsonObject> ParsedIndex,
																		   FString const& IntermediateDir,
																		   FString const& OutputDir,
																		   TSharedPtr<FJsonObject> ConsolidatedOutput)
{
	FJsonDomBuilder::FArray StructList;

	TOptional<TArray<FString>> StructNames = GetNamesFromIndexFile("structs", ParsedIndex);
	if (!StructNames.IsSet())
	{
		return EIntermediateProcessingResult::UnknownError;
	}

	for (const auto& StructName : StructNames.GetValue())
	{
		const FString StructFilePath = IntermediateDir / StructName / StructName + ".json";
		TSharedPtr<FJsonObject> StructJson = ParseStructFile(StructFilePath);
		StructList.Add(MakeShared<FJsonValueObject>(StructJson));
	}

	ConsolidatedOutput->SetField("structs", StructList.AsJsonValue());
	return EIntermediateProcessingResult::Success;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConsolidateEnums(TSharedPtr<FJsonObject> ParsedIndex,
																		 FString const& IntermediateDir,
																		 FString const& OutputDir,
																		 TSharedPtr<FJsonObject> ConsolidatedOutput)
{
	FJsonDomBuilder::FArray EnumList;

	TOptional<TArray<FString>> EnumNames = GetNamesFromIndexFile("enums", ParsedIndex);
	if (!EnumNames.IsSet())
	{
		return EIntermediateProcessingResult::UnknownError;
	}

	for (const auto& EnumName : EnumNames.GetValue())
	{
		const FString EnumFilePath = IntermediateDir / EnumName / EnumName + ".json";
		TSharedPtr<FJsonObject> EnumJson = ParseEnumFile(EnumFilePath);
		EnumList.Add(MakeShared<FJsonValueObject>(EnumJson));
	}

	ConsolidatedOutput->SetField("enums", EnumList.AsJsonValue());
	return EIntermediateProcessingResult::Success;
}

TOptional<TArray<FString>> DocGenMdxOutputProcessor::GetNamesFromIndexFile(const FString& NameType,
																		   TSharedPtr<FJsonObject> ParsedIndex)
{
	if (!ParsedIndex)
	{
		return {};
	}

	const TArray<TSharedPtr<FJsonValue>>* ClassEntries;
	if (!ParsedIndex->TryGetArrayField(NameType, ClassEntries))
	{
		return {};
	}
	TArray<FString> ClassJsonFiles;
	for (const auto& ClassEntry : *ClassEntries)
	{
		TOptional<FString> ClassID = GetObjectStringField(ClassEntry, "id");
		if (ClassID.IsSet())
		{
			ClassJsonFiles.Add(ClassID.GetValue());
		}
	}
	if (ClassJsonFiles.Num())
	{
		return ClassJsonFiles;
	}
	else
	{
		return {};
	}
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::LoadFileToJson(FString const& FilePath)
{
	FString IndexFileString;
	if (!FFileHelper::LoadFileToString(IndexFileString, &FPlatformFileManager::Get().GetPlatformFile(), *FilePath))
	{
		return nullptr;
	}

	TSharedPtr<FJsonStringReader> TopLevelJson = FJsonStringReader::Create(IndexFileString);
	TSharedPtr<FJsonObject> ParsedFile;
	if (!FJsonSerializer::Deserialize<TCHAR>(*TopLevelJson, ParsedFile, FJsonSerializer::EFlags::None))
	{
		return nullptr;
	}
	else
	{
		return ParsedFile;
	}
}
