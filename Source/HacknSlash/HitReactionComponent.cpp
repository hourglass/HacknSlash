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
		// �ǰ� �� �޽� ���� �̺�Ʈ ���ε�
		HitImpactFunction.BindUFunction(this, FName("HitImpactByCurve"));
		HitImpactFunctionEnded.BindUFunction(this, FName("HitImpactTimelineEnded"));

		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// float �Ű� ���� �ϳ��� ������ �Լ��� ���ε�
		HitImpactTimeline->AddInterpFloat(HitImpactCurve, HitImpactFunction);
		HitImpactTimeline->SetTimelineFinishedFunc(HitImpactFunctionEnded);
		HitImpactTimeline->SetLooping(false);
	}

	if (IsValid(HitStunCurve))
	{
		// Binding Timeline Event
		// �ǰ� �� ���� �̺�Ʈ ���ε�
		HitStunFunction.BindUFunction(this, FName("HitStunByCurve"));
		HitStunFunctionEnded.BindUFunction(this, FName("HitStunTimelineEnded"));

		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// float �Ű� ���� �ϳ��� ������ �Լ��� ���ε�
		HitStunTimeline->AddInterpFloat(HitStunCurve, HitStunFunction);
		HitStunTimeline->SetTimelineFinishedFunc(HitStunFunctionEnded);
		HitStunTimeline->SetLooping(false);
	}

	if (IsValid(RimLightingCurve))
	{
		// Binding Timeline Event
		// �������� �̺�Ʈ ���ε�
		RimLightingFunction.BindUFunction(this, FName("RimLightingByCurve"));

		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// float �Ű� ���� �ϳ��� ������ �Լ��� ���ε�
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

// �ǰ� �� Ŀ�� �����͸� ������� �޽��� ������Ű�� �Լ�
void UHitReactionComponent::PlayHitImpact(AActor* Target)
{
	if (IsValid(OwnCharacter))
	{
		// ������ ������ �޽��� ���� ��ġ
		OriginLoc = FVector(0.f, 0.f, OwnCharacter->GetMesh()->GetRelativeLocation().Z);
		
		// ������ ����� �ݴ� ���� ���ϱ�
		FVector TargetDir = OwnCharacter->GetActorLocation() - Target->GetActorLocation();
		TargetDir.Normalize();

		// ���� �� x, y �����θ� ����
		TargetDir = FVector(TargetDir.X, TargetDir.Y, 0.f);
		
		// Ÿ�Ӷ��� ��ǥġ�� ����� �ݴ� ���� * �и��� ����� ����
		TargetLoc = OriginLoc + (TargetDir * ImpactForce);

		// Ÿ�Ӷ��� ����
		HitImpactTimeline->PlayFromStart();
	}
}

void UHitReactionComponent::HitImpactByCurve(float Alpha)
{
	if (IsValid(OwnCharacter))
	{
		// Ŀ�� �����Ϳ� ���� ���� ���ϸ鼭 OriginLoc -> TargetLoc�� �޽��� ��ġ�� �̵�
		FVector FinalLoc = FMath::Lerp(OriginLoc, TargetLoc, Alpha);
		OwnCharacter->GetMesh()->SetRelativeLocation(FinalLoc);
	}
}

void UHitReactionComponent::HitImpactTimelineEnded()
{
	if (IsValid(OwnCharacter))
	{
		// ���� �Ŀ� ���� ��ǥ�� ����
		OwnCharacter->GetMesh()->SetRelativeLocation(OriginLoc);
	}
}


// �ǰ� �� ���� �ð����� �������� �Լ�
void UHitReactionComponent::PlayHitStun()
{
	HitStunTimeline->PlayFromStart();
}

void UHitReactionComponent::HitStunByCurve(float Alpha)
{
	if (IsValid(OwnCharacter))
	{
		// Ŀ�� �����Ϳ� ���� CustomTimeDilation�� ���� ����
		OwnCharacter->CustomTimeDilation = Alpha;
	}
}


void UHitReactionComponent::HitStunTimelineEnded()
{
	if (IsValid(OwnCharacter))
	{
		// Ÿ�Ӷ��� ���� �� ���� �ð����� ����
		OwnCharacter->CustomTimeDilation = 1.f;
	}
}


// �ǰ� �� �������� ȿ�� ����
void UHitReactionComponent::PlayRimLighting()
{
	RimLightingTimeline->PlayFromStart();
}

void UHitReactionComponent::RimLightingByCurve(float Color)
{
	if (IsValid(OwnCharacter))
	{
		// Ŀ�� �����Ϳ� ���� �޽��� ��Ƽ���� ���� ����
		OwnCharacter->GetMesh()->SetVectorParameterValueOnMaterials(FName("HittedColor"), FVector4(Color, 0, 0, 0));
	}
}

