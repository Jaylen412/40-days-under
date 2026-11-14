#include "Game/FortyDaysGameMode.h"
#include "Character/UnderwaterCharacter.h"

AFortyDaysGameMode::AFortyDaysGameMode()
{
	DefaultPawnClass = AUnderwaterCharacter::StaticClass();
}
