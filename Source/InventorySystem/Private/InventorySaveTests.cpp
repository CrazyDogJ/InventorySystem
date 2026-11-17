#if WITH_DEV_AUTOMATION_TESTS
#include "InventoryItemActor.h"
#include "InventoryContainerComponent.h"
#include "InventoryItemDefinition.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryNativeSaveTest, "StreamingLevelSave.Inventory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FInventoryNativeSaveTest::RunTest(const FString&)
{
	auto* Game = NewObject<UGameInstance>(GEngine);
	Game->InitializeStandalone();
	UWorld* World = Game->GetWorld();
	ON_SCOPE_EXIT { Game->Shutdown(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false); };
	auto* Source = World->SpawnActor<AInventoryItemActor>();
	auto* Destination = World->SpawnActor<AInventoryItemActor>();
	auto* Definition = NewObject<UInventoryItemDefinition>(Game);
	Source->ItemEntry.ItemDefinition = Definition;
	auto RoundTrip = [](UObject* From, UObject* To)
	{
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes, true);
		FObjectAndNameAsStringProxyArchive Save(Writer, false);
		Save.ArIsSaveGame = true;
		From->Serialize(Save);
		FMemoryReader Reader(Bytes, true);
		FObjectAndNameAsStringProxyArchive Load(Reader, true);
		Load.ArIsSaveGame = true;
		To->Serialize(Load);
		return !Writer.IsError() && !Reader.IsError();
	};
	TestTrue(TEXT("native item archive"), RoundTrip(Source, Destination));
	TestTrue(TEXT("item definition restored"), Destination->ItemEntry.ItemDefinition == Definition);
	auto* BoxA = NewObject<UInventoryContainerComponent>(Source);
	auto* BoxB = NewObject<UInventoryContainerComponent>(Destination);
	BoxA->ItemList.ItemList.Add(Source->ItemEntry);
	TestTrue(TEXT("native container archive"), RoundTrip(BoxA, BoxB));
	if (TestEqual(TEXT("slot count"), BoxB->ItemList.ItemList.Num(), 1))
		TestTrue(TEXT("container item restored"), BoxB->ItemList.ItemList[0].ItemDefinition == Definition);
	return true;
}
#endif
