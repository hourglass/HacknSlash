// Fill out your copyright notice in the Description page of Project Settings.


#include "HitReactionComponent.h"

// Sets default values for this component's properties
UHitReactionComponent::UHitReactionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;

	// Variable Setting //
	ImpactForce = 50.f;

	// Timeline Setting //
	HitImpactTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("HitImpactTimeline"));
	HitStunTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("HitStunTimeline"));
	RimLightingTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RimLightingTimeline"));

	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		Curve_HitImpact(TEXT("/Game/Curves/Curve_HitImpact.Curve_HitImpact"));

	if (Curve_HitImpact.Succeeded())
	{
		HitImpactCurve = Curve_HitImpact.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		Curve_HitStun(TEXT("/Game/Curves/Curve_HitStun.Curve_HitStun"));

	if (Curve_HitStun.Succeeded())
	{
		HitStunCurve = Curve_HitStun.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		Curve_RimLighting(TEXT("/Game/Curves/Curve_RimLighting.Curve_RimLighting"));

	if (Curve_RimLighting.Succeeded())
	{
		RimLightingCurve = Curve_RimLighting.Object;
	}
}

void UHitReactionComponent::InitializeComponent()
{
	// Character Setting //
	OwnCharacter = Cast<ACharacter>(GetOwner());

	// Timeline Event Setting //
	if (IsValid(HitImpactCurve))
	{
		// Binding Timeline Event
		// 피격 시 메쉬 진동 이벤트 바인딩
		HitImpactFunction.BindUFunction(this, FName("HitImpactByCurve"));
		HitImpactFunctionEnded.BindUFunction(this, FName("HitImpactTimelineEnded"));

		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// float 매개 변수 하나를 가지는 함수를 바인딩
		HitImpactTimeline->AddInterpFloat(HitImpactCurve, HitImpactFunction);
		HitImpactTimeline->SetTimelineFinishedFunc(HitImpactFunctionEnded);
		HitImpactTimeline->SetLooping(false);
	}

	if (IsValid(HitStunCurve))
	{
		// Binding Timeline Event
		// 피격 시 기절 이벤트 바인딩
		HitStunFunction.BindUFunction(this, FName("HitStunByCurve"));
		HitStunFunctionEnded.BindUFunction(this, FName("HitStunTimelineEnded"));

		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// float 매개 변수 하나를 가지는 함수를 바인딩
		HitStunTimeline->AddInterpFloat(HitStunCurve, HitStunFunction);
		HitStunTimeline->SetTimelineFinishedFunc(HitStunFunctionEnded);
		HitStunTimeline->SetLooping(false);
	}

	if (IsValid(RimLightingCurve))
	{
		// Binding Timeline Event
		// 림라이팅 이벤트 바인딩
		RimLightingFunction.BindUFunction(this, FName("RimLightingByCurve"));

		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// float 매개 변수 하나를 가지는 함수를 바인딩
		RimLightingTimeline->AddInterpFloat(RimLightingCurve, RimLightingFunction);
		RimLightingTimeline->SetLooping(false);
	}
}

// Called when the game starts
void UHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void UHitReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// 피격 시 커브 데이터를 기반으로 메쉬를 진동시키는 함수
void UHitReactionComponent::PlayHitImpact(AActor* Target)
{
	if (IsValid(OwnCharacter))
	{
		// 진동을 시작할 메쉬의 원래 위치
		OriginLoc = FVector(0.f, 0.f, OwnCharacter->GetMesh()->GetRelativeLocation().Z);
		
		// 공격한 상대의 반대 방향 구하기
		FVector TargetDir = OwnCharacter->GetActorLocation() - Target->GetActorLocation();
		TargetDir.Normalize();

		// 진동 시 x, y 축으로만 진동
		TargetDir = FVector(TargetDir.X, TargetDir.Y, 0.f);
		
		// 타임라인 목표치를 상대의 반대 방향 * 밀리는 계수로 설정
		TargetLoc = OriginLoc + (TargetDir * ImpactForce);

		// 타임라인 실행
		HitImpactTimeline->PlayFromStart();
	}
}

void UHitReactionComponent::HitImpactByCurve(float Alpha)
{
	if (IsValid(OwnCharacter))
	{
		// 커브 데이터에 의해 값이 변하면서 OriginLoc -> TargetLoc로 메쉬의 위치가 이동
		FVector FinalLoc = FMath::Lerp(OriginLoc, TargetLoc, Alpha);
		OwnCharacter->GetMesh()->SetRelativeLocation(FinalLoc);
	}
}

void UHitReactionComponent::HitImpactTimelineEnded()
{
	if (IsValid(OwnCharacter))
	{
		// 진동 후에 원래 좌표로 복구
		OwnCharacter->GetMesh()->SetRelativeLocation(OriginLoc);
	}
}


// 피격 시 일정 시간동안 느려지는 함수
void UHitReactionComponent::PlayHitStun()
{
	HitStunTimeline->PlayFromStart();
}

void UHitReactionComponent::HitStunByCurve(float Alpha)
{
	if (IsValid(OwnCharacter))
	{
		// 커브 데이터에 따라 CustomTimeDilation의 값이 감소
		OwnCharacter->CustomTimeDilation = Alpha;
	}
}


void UHitReactionComponent::HitStunTimelineEnded()
{
	if (IsValid(OwnCharacter))
	{
		// 타임라인 종료 후 원래 시간으로 복구
		OwnCharacter->CustomTimeDilation = 1.f;
	}
}


// 피격 시 림라이팅 효과 실행
void UHitReactionComponent::PlayRimLighting()
{
	RimLightingTimeline->PlayFromStart();
}

void UHitReactionComponent::RimLightingByCurve(float Color)
{
	if (IsValid(OwnCharacter))
	{
		// 커브 데이터에 따라 메쉬의 머티리얼 값이 변경
		OwnCharacter->GetMesh()->SetVectorParameterValueOnMaterials(FName("HittedColor"), FVector4(Color, 0, 0, 0));
	}
}

