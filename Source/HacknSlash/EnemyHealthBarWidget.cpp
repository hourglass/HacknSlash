// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthBarWidget.h"
#include "Enemy.h"
#include "Components/ProgressBar.h"

void UEnemyHealthBarWidget::NativeConstruct()
{
	HealthBar = Cast<UProgressBar>(GetWidgetFromName("PB_HealthBar"));
	UpdateHealth();
}

void UEnemyHealthBarWidget::BindEnemyHealth(AEnemy* Owner)
{
	if (Owner)
	{
		Enemy = Owner;
		Enemy->OnHealthChanged.AddUObject(this, &UEnemyHealthBarWidget::UpdateHealth);
	}
}

void UEnemyHealthBarWidget::UpdateHealth()
{
	if (Enemy)
	{
		HealthBar->SetPercent(Enemy->GetHealthRatio());
	}
}