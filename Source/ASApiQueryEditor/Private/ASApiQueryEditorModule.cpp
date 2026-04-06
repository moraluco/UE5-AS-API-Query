#include "ASApiQueryEditorModule.h"
#include "ASApiQuerySubsystem.h"

#define LOCTEXT_NAMESPACE "FASApiQueryEditorModule"

DEFINE_LOG_CATEGORY(LogASApiQuery);

class FASApiQueryEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FASApiQueryEditorModule, ASApiQueryEditor)

#undef LOCTEXT_NAMESPACE
