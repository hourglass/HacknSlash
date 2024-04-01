// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HacknSlash.h"
#include "Components/TimeLineComponent.h"
#include "Components/ActorComponent.h"
#include "HitReactionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACKNSLASH_API UHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitReactionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void PlayHitImpact(AActor* Target);
	
	UFUNCTION()
	void PlayHitStun();
	
	UFUNCTION()
	void PlayRimLighting();

private:
	UFUNCTION()
	void HitImpactByCurve(float Alpha);

	UFUNCTION()
	void HitImpactTimelineEnded();

	UFUNCTION()
	void HitStunByCurve(float Alpha);

	UFUNCTION()
	void HitStunTimelineEnded();

	UFUNCTION()
	void RimLightingByCurve(float Color);

	UPROPERTY(BlueprintReadOnly, Category = "Timeline", Meta = (AllowPrivateAccess = true))
	UCurveFloat* HitImpactCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Timeline", Meta = (AllowPrivateAccess = true))
	UCurveFloat* HitStunCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Timeline", Meta = (AllowPrivateAccess = true))
	UCurveFloat* RimLightingCurve;

	UPROPERTY()
	UTimelineComponent* HitImpactTimeline;

	UPROPERTY()
	UTimelineComponent* HitStunTimeline;

	UPROPERTY()
	UTimelineComponent* RimLightingTimeline;

	FOnTimelineFloat HitImpactFunction;
	FOnTimelineEvent HitImpactFunctionEnded;

	FOnTimelineFloat HitStunFunction;
	FOnTimelineEvent HitStunFunctionEnded;

	FOnTimelineFloat RimLightingFunction;

	ACharacter* OwnCharacter;

	FVector OriginLoc;
	FVector TargetLoc;
	float ImpactForce;
};
