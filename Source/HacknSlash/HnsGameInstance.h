// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "HnsGameInstance.generated.h"


UCLASS()
class HACKNSLASH_API UHnsGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UHnsGameInstance();

	struct FWeaponData* GetWeaponDataById(int Id);
	struct FWeaponData* GetWeaponDataByName(FName WeaponName);

	UDataTable* WeaponDataTable;
};
