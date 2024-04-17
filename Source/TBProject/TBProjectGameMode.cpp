// Copyright Epic Games, Inc. All Rights Reserved.

#include "TBProjectGameMode.h"
#include "TBProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATBProjectGameMode::ATBProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
