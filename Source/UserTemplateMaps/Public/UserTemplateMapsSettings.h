// Copyright GeoTech BV

#pragma once

#include "Engine/DeveloperSettings.h"
#include "Templates/TypeHash.h"

#include "UserTemplateMapsSettings.generated.h"

USTRUCT()
struct FUserTemplateMapEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (RelativePath, LongPackageName))
	FDirectoryPath Path;

	UPROPERTY(EditAnywhere)
	FString CategoryName = TEXT("User Templates");

	friend bool operator==(const FUserTemplateMapEntry& Left, const FUserTemplateMapEntry& Right)
	{
		return Left.Path.Path.Equals(Right.Path.Path);
	}

	friend bool operator!=(const FUserTemplateMapEntry& Left, const FUserTemplateMapEntry& Right)
	{
		return !(Left == Right);
	}
};

inline uint32 GetTypeHash(const FUserTemplateMapEntry& Value)
{
	return GetTypeHash(Value.Path.Path);
}

/** */
UCLASS(Config = "Editor", DefaultConfig, MinimalAPI)
class UUserTemplateMapsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Paths", meta = (TitleProperty = "Path.Path.Path"))
	TSet<FUserTemplateMapEntry> Paths;

#if WITH_EDITOR
	USERTEMPLATEMAPS_API virtual FName GetCategoryName() const override;
	USERTEMPLATEMAPS_API virtual FText GetSectionText() const override;
#endif
};
