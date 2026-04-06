#include "ASApiQuerySubsystem.h"
#include "ASApiQueryEditorModule.h"
#include "ASApiQueryHttpServer.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UASApiQuerySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IsRunningCommandlet() || IsRunningDedicatedServer() || FApp::IsUnattended())
	{
		UE_LOG(LogASApiQuery, Log, TEXT("ASApiQuery: skip HTTP (commandlet / dedicated / unattended)"));
		return;
	}

	UE_LOG(LogASApiQuery, Log, TEXT("ASApiQuerySubsystem initializing"));

	Server = MakeUnique<FASApiQueryHttpServer>();
	const int32 Port = ResolvePort();
	if (Server->Start(Port, TEXT("127.0.0.1")))
	{
		WriteInstanceRegistry(Port);
	}
	else
	{
		UE_LOG(LogASApiQuery, Warning, TEXT("ASApiQuery: server failed to start"));
	}
}

void UASApiQuerySubsystem::Deinitialize()
{
	if (Server.IsValid())
	{
		Server->Stop();
		Server.Reset();
	}
	DeleteInstanceRegistry();
	UE_LOG(LogASApiQuery, Log, TEXT("ASApiQuerySubsystem deinitialized"));
	Super::Deinitialize();
}

int32 UASApiQuerySubsystem::ResolvePort() const
{
	FString EnvPort = FPlatformMisc::GetEnvironmentVariable(TEXT("AS_API_QUERY_PORT"));
	if (!EnvPort.IsEmpty())
	{
		const int32 P = FCString::Atoi(*EnvPort);
		if (P > 0)
		{
			return P;
		}
	}
	return 18080;
}

void UASApiQuerySubsystem::WriteInstanceRegistry(int32 Port) const
{
	FString Dir = FPaths::Combine(FPaths::ProjectDir(), TEXT(".as-api-query"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Dir))
	{
		PlatformFile.CreateDirectoryTree(*Dir);
	}

	TSharedPtr<FJsonObject> Info = MakeShareable(new FJsonObject);
	Info->SetNumberField(TEXT("schemaVersion"), 1);
	Info->SetStringField(TEXT("host"), TEXT("127.0.0.1"));
	Info->SetNumberField(TEXT("port"), Port);
	Info->SetStringField(TEXT("basePath"), TEXT("/as-api"));

	FString JsonStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
	FJsonSerializer::Serialize(Info.ToSharedRef(), Writer);

	const FString FilePath = FPaths::Combine(Dir, TEXT("instance.json"));
	FFileHelper::SaveStringToFile(JsonStr, *FilePath);
	UE_LOG(LogASApiQuery, Log, TEXT("ASApiQuery: wrote %s"), *FilePath);
}

void UASApiQuerySubsystem::DeleteInstanceRegistry() const
{
	const FString FilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".as-api-query"), TEXT("instance.json"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*FilePath))
	{
		PlatformFile.DeleteFile(*FilePath);
	}
}
