#include "Narrative/SliceDirector.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool USliceDirector::LoadFromJsonString(const FString& Json, FString& OutError)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutError = TEXT("Expected a valid JSON object.");
		return false;
	}
	const TArray<TSharedPtr<FJsonValue>>* Values;
	if (!Root->TryGetArrayField(TEXT("beats"), Values) || Values->IsEmpty())
	{
		OutError = TEXT("beats must be a non-empty array.");
		return false;
	}
	TArray<FSliceBeat> Parsed;
	TSet<FName> Ids;
	for (const TSharedPtr<FJsonValue>& Value : *Values)
	{
		if (!Value.IsValid() || Value->Type != EJson::Object)
		{
			OutError = TEXT("Each beat must be an object.");
			return false;
		}
		const TSharedPtr<FJsonObject> Object = Value->AsObject();
		FSliceBeat Beat;
		FString Id, Event;
		auto StringField = [&Object](const TCHAR* Key, FString& Out)
		{
			const TSharedPtr<FJsonValue> Field = Object->TryGetField(Key);
			return Field.IsValid() && Field->Type == EJson::String && Field->TryGetString(Out);
		};
		const TSharedPtr<FJsonValue> Time = Object->TryGetField(TEXT("timeoutSeconds"));
		if (!StringField(TEXT("id"), Id) || !StringField(TEXT("speaker"), Beat.Speaker) ||
			!StringField(TEXT("line"), Beat.Line) || !StringField(TEXT("completeOnEvent"), Event) || !Time.IsValid() ||
			Time->Type != EJson::Number || !Time->TryGetNumber(Beat.TimeoutSeconds) ||
			!FMath::IsFinite(Beat.TimeoutSeconds) || Beat.TimeoutSeconds <= 0.0)
		{
			OutError = TEXT("Beat requires string id, speaker, line, completeOnEvent and a finite positive numeric "
							"timeoutSeconds.");
			return false;
		}
		Beat.Id = FName(*Id.TrimStartAndEnd());
		Beat.CompleteOnEvent = FName(*Event.TrimStartAndEnd());
		if (Beat.Id.IsNone() || Beat.CompleteOnEvent.IsNone() || Ids.Contains(Beat.Id))
		{
			OutError = TEXT("Beat ids must be unique and non-empty; completion events must be non-empty.");
			return false;
		}
		Ids.Add(Beat.Id);
		Parsed.Add(MoveTemp(Beat));
	}
	// Commit only after every beat validates: rejected loads preserve elapsed time too.
	Beats = MoveTemp(Parsed);
	CurrentIndex = 0;
	++Revision;
	OutError.Reset();
	StartCurrentBeat();
	return true;
}

bool USliceDirector::LoadFromFile(const FString& Path, FString& OutError)
{
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *Path))
	{
		OutError = FString::Printf(TEXT("Could not read narrative file: %s"), *Path);
		return false;
	}
	return LoadFromJsonString(Json, OutError);
}

FName USliceDirector::CurrentBeatId() const
{
	return Beats.IsValidIndex(CurrentIndex) ? Beats[CurrentIndex].Id : NAME_None;
}

FString USliceDirector::CurrentSpeaker() const
{
	return Beats.IsValidIndex(CurrentIndex) ? Beats[CurrentIndex].Speaker : FString();
}

FString USliceDirector::CurrentLine() const
{
	return Beats.IsValidIndex(CurrentIndex) ? Beats[CurrentIndex].Line : FString();
}

void USliceDirector::StartCurrentBeat()
{
	RemainingSeconds = Beats.IsValidIndex(CurrentIndex) ? Beats[CurrentIndex].TimeoutSeconds : 0.0;
	if (Beats.IsValidIndex(CurrentIndex))
	{
		OnBeatStarted.Broadcast(Beats[CurrentIndex].Id);
	}
}

void USliceDirector::AdvanceBeat()
{
	++CurrentIndex;
	++Revision;
	StartCurrentBeat();
}

bool USliceDirector::NotifyEvent(FName Event)
{
	if (!Beats.IsValidIndex(CurrentIndex) || Event != Beats[CurrentIndex].CompleteOnEvent)
	{
		return false;
	}
	AdvanceBeat();
	return true;
}

void USliceDirector::Tick(double RealSeconds)
{
	if (!FMath::IsFinite(RealSeconds) || RealSeconds <= 0.0)
	{
		return;
	}
	while (Beats.IsValidIndex(CurrentIndex))
	{
		if (RealSeconds < RemainingSeconds)
		{
			RemainingSeconds -= RealSeconds;
			return;
		}
		RealSeconds -= RemainingSeconds;
		const uint64 ExpectedRevision = Revision + 1;
		AdvanceBeat();
		// A listener handled a new event or loaded a new slice; don't apply old time to it.
		if (Revision != ExpectedRevision || RealSeconds == 0.0)
		{
			return;
		}
	}
}
