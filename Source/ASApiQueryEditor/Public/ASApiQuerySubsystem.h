#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EditorSubsystem.h"
#include "ASApiQuerySubsystem.generated.h"

class FASApiQueryHttpServer;

/** Editor-only HTTP service for AS API queries (independent from SoftUEBridge). */
UCLASS()
class UASApiQuerySubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	TUniquePtr<FASApiQueryHttpServer> Server;

	int32 ResolvePort() const;
	void WriteInstanceRegistry(int32 Port) const;
	void DeleteInstanceRegistry() const;
};
