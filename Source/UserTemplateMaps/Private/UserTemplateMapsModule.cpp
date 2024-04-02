// Copyright GeoTech BV

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Modules/ModuleInterface.h"
#include "UnrealEdGlobals.h"
#include "UserTemplateMapsLog.h"
#include "UserTemplateMapsSettings.h"

DEFINE_LOG_CATEGORY(LogUserTemplateMaps);

#define LOCTEXT_NAMESPACE "UserTemplateMapsModule"

class FUserTemplateMapsModule
	: public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		if (UUserTemplateMapsSettings* Settings = GetMutableDefault<UUserTemplateMapsSettings>())
		{
			Settings->OnSettingChanged().AddRaw(this, &FUserTemplateMapsModule::OnSettingsChanged);
		}
		
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUserTemplateMapsModule::PostEngineInit);
	}

	virtual void ShutdownModule() override
	{
		if (UUserTemplateMapsSettings* Settings = GetMutableDefault<UUserTemplateMapsSettings>())
		{
			Settings->OnSettingChanged().RemoveAll(this);
		}

		FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	}

private:
	void OnSettingsChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
	{
		// Effectively refresh maps
		RegisterLevelTemplates();
	}

	void PostEngineInit()
	{
		if (FSlateApplication::IsInitialized())
		{
			RegisterLevelTemplates();
		}
	}

	void RegisterLevelTemplates()
	{
		if (GUnrealEd)
		{
			if (IAssetRegistry* AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).TryGet())
			{
				const UUserTemplateMapsSettings* Settings = GetDefault<UUserTemplateMapsSettings>();
				check(Settings);

				TArray<FTemplateMapInfo> TemplateMaps;
				for (const FUserTemplateMapEntry& TemplatePath : Settings->Paths)
				{
					TArray<FString> LevelTemplatePaths;
					AssetRegistry->GetSubPaths(TemplatePath.Path.Path, LevelTemplatePaths, false);

					for (const FString& LevelTemplatePath : LevelTemplatePaths)
					{
						FString LevelTemplateName = FPaths::GetPathLeaf(LevelTemplatePath);

						TArray<FAssetData> LevelTemplateAssetData;				
						if (AssetRegistry->GetAssetsByPath(FName(LevelTemplatePath), LevelTemplateAssetData, true))
						{
							FAssetData LevelAsset;
							FAssetData ThumbnailAsset;
							
							for (FAssetData& AssetData : LevelTemplateAssetData)
							{
								if (LevelAsset.IsValid() && ThumbnailAsset.IsValid())
								{
									break;
								}

								if (!AssetData.AssetName.ToString().Contains(LevelTemplateName))
								{
									continue;
								}
								
								if (AssetData.AssetClassPath == UWorld::StaticClass()->GetClassPathName())
								{
									LevelAsset = AssetData;
								}
								else if (AssetData.AssetClassPath == UTexture2D::StaticClass()->GetClassPathName())
								{
									ThumbnailAsset = AssetData;
								}
							}

							if (!LevelAsset.IsValid())
							{
								UE_LOG(LogUserTemplateMaps, Warning, TEXT(
									"A map template path was found, but it didn't contain a matching level with the name '%s'. "
									"Ensure a level exists, and name it appropriately (ie. '%s')"),
									*LevelTemplateName,
									*FString::Printf(TEXT("L_%s"), *LevelTemplateName));
							}
							else
							{
								UTexture2D* ThumbnailTexture = nullptr;

								// Thumbnail will be used if found, but is not essential
								if (!ThumbnailAsset.IsValid())
								{
									UE_LOG(LogUserTemplateMaps, Warning, TEXT(
										"A map template path was found, but it didn't contain a matching thumbnail texture with the name '%s'. "
										"To use a custom thumbnail, ensure a texture exists, and name it appropriately (ie. '%s')"),
										*LevelTemplateName,
										*FString::Printf(TEXT("T_%s"), *LevelTemplateName));
								}

								if (!GUnrealEd->IsTemplateMap(LevelAsset.GetObjectPathString()))
								{
									FString LevelTemplateDisplayName = FName::NameToDisplayString(LevelTemplateName, false);

									FTemplateMapInfo& MapTemplate = TemplateMaps.Emplace_GetRef();
									MapTemplate.Category = TemplatePath.CategoryName;
									MapTemplate.DisplayName = FText::FromString(LevelTemplateDisplayName);
									MapTemplate.Map = LevelAsset.GetSoftObjectPath();

									if (ThumbnailAsset.IsValid())
									{
										MapTemplate.ThumbnailTexture = ThumbnailAsset.GetSoftObjectPath();
									}
								}
							}
						}
					}

					GUnrealEd->AppendTemplateMaps(TemplateMaps);
				}
			}
		}
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUserTemplateMapsModule, UserTemplateMaps)
