#include "ASApiQueryHttpServer.h"
#include "ASApiQueryEditorModule.h"
#include "AngelscriptInclude.h"
#include "AngelscriptManager.h"
#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "HttpPath.h"
#include "HttpServerResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	void SendCors(TUniquePtr<FHttpServerResponse>& Resp)
	{
		Resp->Headers.Add(TEXT("Access-Control-Allow-Origin"), {TEXT("*")});
		Resp->Headers.Add(TEXT("Access-Control-Allow-Methods"), {TEXT("GET, POST, OPTIONS")});
		Resp->Headers.Add(TEXT("Access-Control-Allow-Headers"), {TEXT("Content-Type")});
	}

	void SendJson(const FHttpResultCallback& OnComplete, const TSharedPtr<FJsonObject>& Json, EHttpServerResponseCodes Code = EHttpServerResponseCodes::Ok)
	{
		FString Out;
		TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&Out);
		FJsonSerializer::Serialize(Json.ToSharedRef(), W);
		TUniquePtr<FHttpServerResponse> Resp = FHttpServerResponse::Create(Out, TEXT("application/json"));
		SendCors(Resp);
		Resp->Code = Code;
		OnComplete(MoveTemp(Resp));
	}

	void ProcessQueryOnGameThread(const FString& Body, const FHttpResultCallback& OnComplete)
	{
		TSharedPtr<FJsonObject> Root;
		{
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
			if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
			{
				TSharedPtr<FJsonObject> Err = MakeShareable(new FJsonObject);
				Err->SetNumberField(TEXT("schemaVersion"), 1);
				Err->SetBoolField(TEXT("ok"), false);
				TSharedPtr<FJsonObject> E = MakeShareable(new FJsonObject);
				E->SetStringField(TEXT("code"), TEXT("invalid_json"));
				E->SetStringField(TEXT("message"), TEXT("Request body is not valid JSON"));
				Err->SetObjectField(TEXT("error"), E);
				SendJson(OnComplete, Err, EHttpServerResponseCodes::BadRequest);
				return;
			}
		}

		FString Action;
		if (!Root->TryGetStringField(TEXT("action"), Action))
		{
			TSharedPtr<FJsonObject> Err = MakeShareable(new FJsonObject);
			Err->SetNumberField(TEXT("schemaVersion"), 1);
			Err->SetBoolField(TEXT("ok"), false);
			TSharedPtr<FJsonObject> E = MakeShareable(new FJsonObject);
			E->SetStringField(TEXT("code"), TEXT("invalid_json"));
			E->SetStringField(TEXT("message"), TEXT("Missing action"));
			Err->SetObjectField(TEXT("error"), E);
			SendJson(OnComplete, Err, EHttpServerResponseCodes::BadRequest);
			return;
		}

		if (Action == TEXT("list_types"))
		{
			FString Filter;
			Root->TryGetStringField(TEXT("filter"), Filter);

			if (!FAngelscriptManager::IsInitialized())
			{
				TSharedPtr<FJsonObject> Err = MakeShareable(new FJsonObject);
				Err->SetNumberField(TEXT("schemaVersion"), 1);
				Err->SetBoolField(TEXT("ok"), false);
				TSharedPtr<FJsonObject> E = MakeShareable(new FJsonObject);
				E->SetStringField(TEXT("code"), TEXT("not_initialized"));
				E->SetStringField(TEXT("message"), TEXT("Angelscript manager is not initialized"));
				Err->SetObjectField(TEXT("error"), E);
				SendJson(OnComplete, Err, EHttpServerResponseCodes::Ok);
				return;
			}

			asIScriptEngine* Eng = FAngelscriptManager::Get().GetScriptEngine();
			if (!Eng)
			{
				TSharedPtr<FJsonObject> Err = MakeShareable(new FJsonObject);
				Err->SetNumberField(TEXT("schemaVersion"), 1);
				Err->SetBoolField(TEXT("ok"), false);
				TSharedPtr<FJsonObject> E = MakeShareable(new FJsonObject);
				E->SetStringField(TEXT("code"), TEXT("internal"));
				E->SetStringField(TEXT("message"), TEXT("Script engine is null"));
				Err->SetObjectField(TEXT("error"), E);
				SendJson(OnComplete, Err, EHttpServerResponseCodes::Ok);
				return;
			}

			TArray<TSharedPtr<FJsonValue>> TypesArr;
			const asUINT n = Eng->GetObjectTypeCount();
			for (asUINT i = 0; i < n; ++i)
			{
				asITypeInfo* Ti = Eng->GetObjectTypeByIndex(i);
				if (!Ti)
				{
					continue;
				}
				const char* N = Ti->GetName();
				const FString Name = N ? ANSI_TO_TCHAR(N) : FString();
				if (!Filter.IsEmpty() && !Name.Contains(Filter, ESearchCase::IgnoreCase))
				{
					continue;
				}
				TSharedPtr<FJsonObject> O = MakeShareable(new FJsonObject);
				O->SetStringField(TEXT("name"), Name);
				TypesArr.Add(MakeShareable(new FJsonValueObject(O)));
			}

			TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
			Data->SetStringField(TEXT("action"), TEXT("list_types"));
			Data->SetArrayField(TEXT("types"), TypesArr);
			Data->SetNumberField(TEXT("total"), TypesArr.Num());

			TSharedPtr<FJsonObject> Ok = MakeShareable(new FJsonObject);
			Ok->SetNumberField(TEXT("schemaVersion"), 1);
			Ok->SetBoolField(TEXT("ok"), true);
			Ok->SetObjectField(TEXT("data"), Data);
			SendJson(OnComplete, Ok, EHttpServerResponseCodes::Ok);
			return;
		}

		TSharedPtr<FJsonObject> Err = MakeShareable(new FJsonObject);
		Err->SetNumberField(TEXT("schemaVersion"), 1);
		Err->SetBoolField(TEXT("ok"), false);
		TSharedPtr<FJsonObject> E = MakeShareable(new FJsonObject);
		E->SetStringField(TEXT("code"), TEXT("unknown_action"));
		E->SetStringField(TEXT("message"), FString::Printf(TEXT("Unknown action: %s"), *Action));
		Err->SetObjectField(TEXT("error"), E);
		SendJson(OnComplete, Err, EHttpServerResponseCodes::Ok);
	}
}

FASApiQueryHttpServer::FASApiQueryHttpServer() = default;

FASApiQueryHttpServer::~FASApiQueryHttpServer()
{
	Stop();
}

bool FASApiQueryHttpServer::Start(int32 Port, const FString& BindAddress)
{
	if (bIsRunning)
	{
		return true;
	}

	ServerPort = Port;
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	HttpRouter = HttpServerModule.GetHttpRouter(ServerPort);
	if (!HttpRouter.IsValid())
	{
		UE_LOG(LogASApiQuery, Error, TEXT("ASApiQuery: failed to get HTTP router for port %d"), ServerPort);
		Status = EASApiQueryServerStatus::Error;
		return false;
	}

	HealthRouteHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/as-api/health")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FASApiQueryHttpServer::HandleHealth));

	QueryRouteHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/as-api/v1/query")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FASApiQueryHttpServer::HandleQuery));

	if (!HealthRouteHandle.IsValid() || !QueryRouteHandle.IsValid())
	{
		UE_LOG(LogASApiQuery, Error, TEXT("ASApiQuery: failed to bind routes"));
		Status = EASApiQueryServerStatus::Error;
		return false;
	}

	HttpServerModule.StartAllListeners();
	bIsRunning = true;
	Status = EASApiQueryServerStatus::Running;
	UE_LOG(LogASApiQuery, Log, TEXT("ASApiQuery HTTP listening on http://%s:%d"), *BindAddress, ServerPort);
	return true;
}

void FASApiQueryHttpServer::Stop()
{
	if (!bIsRunning)
	{
		return;
	}

	if (HttpRouter.IsValid())
	{
		if (HealthRouteHandle.IsValid())
		{
			HttpRouter->UnbindRoute(HealthRouteHandle);
		}
		if (QueryRouteHandle.IsValid())
		{
			HttpRouter->UnbindRoute(QueryRouteHandle);
		}
	}

	bIsRunning = false;
	Status = EASApiQueryServerStatus::Stopped;
	UE_LOG(LogASApiQuery, Log, TEXT("ASApiQuery HTTP stopped"));
}

bool FASApiQueryHttpServer::HandleHealth(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("name"), TEXT("as-api-query"));
	Data->SetStringField(TEXT("version"), TEXT("0.1.0"));

	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
	Root->SetNumberField(TEXT("schemaVersion"), 1);
	Root->SetBoolField(TEXT("ok"), true);
	Root->SetObjectField(TEXT("data"), Data);
	SendJson(OnComplete, Root, EHttpServerResponseCodes::Ok);
	return true;
}

bool FASApiQueryHttpServer::HandleQuery(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	FString Body;
	if (Request.Body.Num() > 0)
	{
		FUTF8ToTCHAR Convert(reinterpret_cast<const ANSICHAR*>(Request.Body.GetData()), Request.Body.Num());
		Body = FString(Convert.Length(), Convert.Get());
	}

	AsyncTask(ENamedThreads::GameThread, [Body, OnComplete]() { ProcessQueryOnGameThread(Body, OnComplete); });
	return true;
}
