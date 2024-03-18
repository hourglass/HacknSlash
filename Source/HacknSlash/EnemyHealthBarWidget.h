// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class HACKNSLASH_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void BindEnemyHealth(class AEnemy* Owner);
	void UpdateHealth();

	class AEnemy* Enemy;

	UPROPERTY()
	class UProgressBar* HealthBar;
};
