#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Narrative/FootagePlaylist.h"
#include "Narrative/SliceDirector.h"
#include "UObject/StrongObjectPtr.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace FootageTest
{
constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFootageParse, "FortyDaysUnder.Footage.Parse", FootageTest::Flags)
bool FFootageParse::RunTest(const FString& Parameters)
{
	TArray<FFootageClip> Clips;
	FString Error;
	TestTrue(TEXT("valid"),
		FFootagePlaylist::ParseJson(
			TEXT(R"({"clips":[{"beat":"a","file":"Movies/a.mp4"},{"beat":"b","file":"Movies/b.mp4"}]})"), Clips, Error));
	TestEqual(TEXT("count"), Clips.Num(), 2);
	TestEqual(TEXT("order"), Clips[1].BeatId, FName("b"));
	TestEqual(TEXT("file"), Clips[0].File, FString(TEXT("Movies/a.mp4")));
	TestFalse(TEXT("not json"), FFootagePlaylist::ParseJson(TEXT("nope"), Clips, Error));
	TestFalse(TEXT("no clips"), FFootagePlaylist::ParseJson(TEXT(R"({"x":[]})"), Clips, Error));
	TestFalse(TEXT("empty file"), FFootagePlaylist::ParseJson(TEXT(R"({"clips":[{"beat":"a","file":""}]})"), Clips, Error));
	TestEqual(TEXT("cleared on failure"), Clips.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFootagePlayback, "FortyDaysUnder.Footage.Playback", FootageTest::Flags)
bool FFootagePlayback::RunTest(const FString& Parameters)
{
	const TArray<FFootageClip> All = {{"a", "1.mp4"}, {"b", "2.mp4"}, {"c", "3.mp4"}};
	const TArray<FFootageClip> Kept =
		FFootagePlaylist::KeepExisting(All, TEXT("Base"), [](const FString& Path) { return !Path.EndsWith(TEXT("2.mp4")); });
	TestEqual(TEXT("missing dropped"), Kept.Num(), 2);
	TestEqual(TEXT("order kept"), Kept[1].BeatId, FName("c"));

	FFootagePlaylist P;
	TestTrue(TEXT("empty is finished"), P.IsFinished());
	TestNull(TEXT("empty has no clip"), P.Current());
	P.Reset(Kept);
	TestEqual(TEXT("first"), P.Current()->BeatId, FName("a"));
	TestTrue(TEXT("advance"), P.Advance());
	TestEqual(TEXT("second"), P.Current()->BeatId, FName("c"));
	TestFalse(TEXT("end"), P.Advance());
	TestTrue(TEXT("finished"), P.IsFinished());
	TestFalse(TEXT("advance past end is safe"), P.Advance());
	TestEqual(TEXT("index clamped"), P.CurrentIndex(), 2);
	P.Reset(Kept);
	P.SkipAll();
	TestTrue(TEXT("skip all"), P.IsFinished());
	return true;
}

/** The shipped manifest parses, names only real slice beats in slice order, and points at .mp4 files. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFootageManifest, "FortyDaysUnder.Footage.Manifest", FootageTest::Flags)
bool FFootageManifest::RunTest(const FString& Parameters)
{
	const FString Content = FPaths::ProjectContentDir();
	FString Json;
	if (!TestTrue(TEXT("manifest exists"),
			FFileHelper::LoadFileToString(Json, *(Content / TEXT("Data/Narrative/opening_slice_footage.json")))))
	{
		return false;
	}
	TArray<FFootageClip> Clips;
	FString Error;
	if (!TestTrue(TEXT("manifest parses"), FFootagePlaylist::ParseJson(Json, Clips, Error)))
	{
		AddError(Error);
		return false;
	}
	TStrongObjectPtr<USliceDirector> Director(NewObject<USliceDirector>());
	if (!TestTrue(TEXT("slice loads"), Director->LoadFromFile(Content / TEXT("Data/Narrative/opening_slice.json"), Error)))
	{
		return false;
	}
	double LastStart = -1.0;
	for (const FFootageClip& Clip : Clips)
	{
		const double Start = Director->BeatStartSeconds(Clip.BeatId);
		TestTrue(FString::Printf(TEXT("%s is a slice beat"), *Clip.BeatId.ToString()), Start >= 0.0);
		TestTrue(FString::Printf(TEXT("%s in slice order"), *Clip.File), Start >= LastStart);
		TestTrue(FString::Printf(TEXT("%s is mp4"), *Clip.File), Clip.File.EndsWith(TEXT(".mp4")));
		LastStart = Start;
	}
	const TArray<FFootageClip> Present = FFootagePlaylist::KeepExisting(
		Clips, Content, [](const FString& Path) { return IFileManager::Get().FileExists(*Path); });
	AddInfo(FString::Printf(TEXT("%d of %d footage clips present"), Present.Num(), Clips.Num()));
	TestTrue(TEXT("at least one clip present"), Present.Num() > 0);
	return true;
}
#endif
