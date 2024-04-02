// Copyright GeoTech BV

#include "UserTemplateMapsSettings.h"

FName UUserTemplateMapsSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FText UUserTemplateMapsSettings::GetSectionText() const
{
	return NSLOCTEXT("UserTemplateMaps", "UserTemplateMapsSettingsSection", "Map Templates");
}
