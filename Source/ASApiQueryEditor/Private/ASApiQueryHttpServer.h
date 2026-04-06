#pragma once

#include "CoreMinimal.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"

enum class EASApiQueryServerStatus : uint8
{
	Stopped,
	Running,
	Error,
};

/** Minimal HTTP server: GET /as-api/health, POST /as-api/v1/query */
class FASApiQueryHttpServer
{
public:
	FASApiQueryHttpServer();
	~FASApiQueryHttpServer();

	bool Start(int32 Port = 18080, const FString& BindAddress = TEXT("127.0.0.1"));
	void Stop();

	bool IsRunning() const { return bIsRunning; }
	int32 GetPort() const { return ServerPort; }

private:
	TSharedPtr<IHttpRouter> HttpRouter;
	FHttpRouteHandle HealthRouteHandle;
	FHttpRouteHandle QueryRouteHandle;

	bool bIsRunning = false;
	int32 ServerPort = 18080;
	EASApiQueryServerStatus Status = EASApiQueryServerStatus::Stopped;

	bool HandleHealth(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool HandleQuery(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
};
