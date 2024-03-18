// Fill out your copyright notice in the Description page of Project Settings.


#include "HnsGameModeBase.h"
#include "HnsPlayerController.h"
#include "HnsCharacter.h"


AHnsGameModeBase::AHnsGameModeBase()
{
	PlayerControllerClass = AHnsPlayerController::StaticClass();
	DefaultPawnClass = AHnsCharacter::StaticClass();
}