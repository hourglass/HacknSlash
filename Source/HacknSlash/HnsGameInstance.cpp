// Fill out your copyright notice in the Description page of Project Settings.


#include "HnsGameInstance.h"
#include "Weapon.h"

UHnsGameInstance::UHnsGameInstance()
{
	static ConstructorHelpers::FObjectFinder<UDataTable>
		DT_WeaponData(TEXT("/Game/DataTables/DT_WeaponData.DT_WeaponData"));

	if (DT_WeaponData.Succeeded())
	{
		WeaponDataTable = DT_WeaponData.Object;
	}

	check(WeaponDataTable->GetRowMap().Num() > 0);
}

struct FWeaponData* UHnsGameInstance::GetWeaponDataById(int Id)
{
	auto WeaponData = WeaponDataTable->FindRow<FWeaponData>(*FString::FromInt(Id), "");

	return WeaponData;
}

struct FWeaponData* UHnsGameInstance::GetWeaponDataByName(FName WeaponName)
{
	auto WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponName, TEXT(""));

	return WeaponData;
}