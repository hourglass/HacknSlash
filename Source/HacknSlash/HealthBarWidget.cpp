// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthBarWidget.h"
#include "HnsCharacter.h"
#include "Components/ProgressBar.h"

void UHealthBarWidget::NativeConstruct()
{
	HealthBar = Cast<UProgressBar>(GetWidgetFromName("PB_HealthBar"));
	UpdateHealth();
}

void UHealthBarWidget::BindCharacterHealth(AHnsCharacter* Owner)
{
	if (Owner)
	{
		MyCharacter = Owner;
		MyCharacter->OnHealthChanged.AddUObject(this, &UHealthBarWidget::UpdateHealth);
	}
}

void UHealthBarWidget::UpdateHealth()
{
	if (MyCharacter)
	{
		HealthBar->SetPercent(MyCharacter->GetHealthRatio());
	}
}