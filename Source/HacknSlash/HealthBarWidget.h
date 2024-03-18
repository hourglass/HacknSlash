// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class HACKNSLASH_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void BindCharacterHealth(class AHnsCharacter* Owner);
	void UpdateHealth();

	class AHnsCharacter* MyCharacter;

	UPROPERTY()
	class UProgressBar* HealthBar;
};
