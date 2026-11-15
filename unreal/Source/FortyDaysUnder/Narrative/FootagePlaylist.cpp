#include "Narrative/FootagePlaylist.h"

#include "Dom/JsonObject.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FFootagePlaylist::ParseJson(const FString& Json, TArray<FFootageClip>& OutClips, FString& OutError)
{
	OutClips.Reset();
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutError = TEXT("footage manifest is not valid JSON");
		return false;
	}
	const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
	if (!Root->TryGetArrayField(TEXT("clips"), Entries))
	{
		OutError = TEXT("footage manifest has no \"clips\" array");
		return false;
	}
	for (int32 i = 0; i < Entries->Num(); ++i)
	{
		const TSharedPtr<FJsonObject>* Entry = nullptr;
		FString Beat;
		FString File;
		if (!(*Entries)[i]->TryGetObject(Entry) || !(*Entry)->TryGetStringField(TEXT("beat"), Beat) ||
			!(*Entry)->TryGetStringField(TEXT("file"), File) || Beat.IsEmpty() || File.IsEmpty())
		{
			OutError = FString::Printf(TEXT("clip %d needs non-empty \"beat\" and \"file\""), i);
			OutClips.Reset();
			return false;
		}
		OutClips.Add({FName(*Beat), File});
	}
	return true;
}

TArray<FFootageClip> FFootagePlaylist::KeepExisting(
	const TArray<FFootageClip>& Clips, const FString& BaseDir, TFunctionRef<bool(const FString&)> Exists)
{
	TArray<FFootageClip> Kept;
	for (const FFootageClip& Clip : Clips)
	{
		if (Exists(FPaths::Combine(BaseDir, Clip.File)))
		{
			Kept.Add(Clip);
		}
	}
	return Kept;
}
