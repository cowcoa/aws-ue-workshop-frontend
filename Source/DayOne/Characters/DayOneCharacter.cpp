// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOneCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/DayOne.h"
#include "DayOne/Components/CombatComponent.h"
#include "DayOne/GameModes/BattlefieldGameMode.h"
#include "DayOne/PlayerController/DayOnePlayerController.h"
#include "DayOne/PlayerStates/DayOnePlayerState.h"
#include "DayOne/UI/OverheadWidget.h"
#include "DayOne/Weapons/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADayOneCharacter::ADayOneCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCharCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCharCamera"));
	FollowCharCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCharCamera->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	
	bUseControllerRotationYaw = false;
}

void ADayOneCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;
}

bool ADayOneCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ADayOneCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ADayOneCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ADayOneCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

UCameraComponent* ADayOneCharacter::GetFollowCamera() const
{
	return FollowCharCamera;
}

void ADayOneCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ADayOneCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ADayOneCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayElimMontage, Elim flag is: %s"), bElimmed ? *FString("true") : *FString("false"));
		}
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ADayOneCharacter::UpdateHUDHealth()
{
	DayOnePlayerController = DayOnePlayerController == nullptr ? Cast<ADayOnePlayerController>(Controller) : DayOnePlayerController;
	if (DayOnePlayerController)
	{
		DayOnePlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ADayOneCharacter::Elim(bool bPlayerLeftGame)
{
	if (Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	
	MulticastElim(bPlayerLeftGame);

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinished,
		ElimDelay
	);
}

void ADayOneCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	if (DayOnePlayerController)
	{
		DayOnePlayerController->SetHUDWeaponAmmo(0);
	}
	
	bElimmed = true;
	PlayElimMontage();

	// Disable character movement
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TArray<USceneComponent*> MeshChildren;
	GetMesh()->GetChildrenComponents(false, MeshChildren);
	for (auto Child : MeshChildren)
	{
		USkeletalMeshComponent* MeshChild = Cast<USkeletalMeshComponent>(Child);
		if (MeshChild != nullptr)
		{
			if (Combat->EquippedWeapon == nullptr || MeshChild != Combat->EquippedWeapon->GetWeaponMesh())
			{
				MeshChild->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

// Called when the game starts or when spawned
void ADayOneCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
	
	auto OverheadWidgetObject = Cast<UOverheadWidget>(OverheadWidget->GetUserWidgetObject());
	if (OverheadWidgetObject)
	{
		OverheadWidgetObject->ShowPlayerNetRole(this);
	}
}

void ADayOneCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ThisClass, Health);
	DOREPLIFETIME(ThisClass, bDisableGameplay);
}

void ADayOneCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void ADayOneCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ADayOneCharacter::Move(const FVector2D& MoveValue)
{
	if (Controller != nullptr)
	{
		// Forward/Backward direction
		if (MoveValue.Y != 0.f)
		{
			MoveForward(MoveValue.Y);
		}
 
		// Right/Left direction
		if (MoveValue.X != 0.f)
		{
			MoveRight(MoveValue.X);
		}
	}
}

void ADayOneCharacter::Look(const FVector2D& LookValue)
{
	Turn(LookValue.X);
	LookUp(LookValue.Y);
}

void ADayOneCharacter::JumpUp()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ADayOneCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		ServerEquipButtonPressed();
	}
}

void ADayOneCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ADayOneCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ADayOneCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ADayOneCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ADayOneCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ADayOneCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReloadButtonPressed"));
		Combat->Reload();
	}
}

void ADayOneCharacter::ReloadButtonReleased()
{
}

void ADayOneCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ADayOneCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ADayOneCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ADayOneCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ADayOneCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	if (Health == 0.f)
	{
		ABattlefieldGameMode *BattlefieldGameMode = GetWorld()->GetAuthGameMode<ABattlefieldGameMode>();
		if (BattlefieldGameMode)
		{
			DayOnePlayerController = DayOnePlayerController == nullptr ? Cast<ADayOnePlayerController>(Controller) : DayOnePlayerController;
			ADayOnePlayerController* AttackerController = Cast<ADayOnePlayerController>(InstigatorController);
			BattlefieldGameMode->PlayerEliminated(this, DayOnePlayerController, AttackerController);
		}
	}
	//UpdateHUDHealth();
	//PlayHitReactMontage();
}

void ADayOneCharacter::OnPlayerStateInitialized()
{
	DayOnePlayerState->AddToScore(0.f);
}

void ADayOneCharacter::PollInit()
{
	if (DayOnePlayerState == nullptr)
	{
		DayOnePlayerState = GetPlayerState<ADayOnePlayerState>();
		if (DayOnePlayerState)
		{
			OnPlayerStateInitialized();
		}
	}
}

void ADayOneCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ADayOneCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
    {
    	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
    	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
    	AddMovementInput(Direction, Value);
    }
}

void ADayOneCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ADayOneCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ADayOneCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

float ADayOneCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ADayOneCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth && Health != 0.f)
	{
		PlayHitReactMontage();
	}
}

void ADayOneCharacter::ElimTimerFinished()
{
	ABattlefieldGameMode *BattlefieldGameMode = GetWorld()->GetAuthGameMode<ABattlefieldGameMode>();
	if (BattlefieldGameMode)
	{
		BattlefieldGameMode->RequestRespawn(this, Controller);
	}
}

void ADayOneCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
	}
}

void ADayOneCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ADayOneCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;

	TArray<USceneComponent*> MeshChildren;
	GetMesh()->GetChildrenComponents(false, MeshChildren);
	
	if ((FollowCharCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		for (auto Child : MeshChildren)
		{
			USkeletalMeshComponent* MeshChild = Cast<USkeletalMeshComponent>(Child);
			if (MeshChild != nullptr && Combat->EquippedWeapon != nullptr && MeshChild != Combat->EquippedWeapon->GetWeaponMesh())
			{
				MeshChild->SetVisibility(false);
			}
		}
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		for (auto Child : MeshChildren)
		{
			USkeletalMeshComponent* MeshChild = Cast<USkeletalMeshComponent>(Child);
			if (MeshChild != nullptr && Combat->EquippedWeapon != nullptr && MeshChild != Combat->EquippedWeapon->GetWeaponMesh())
			{
				MeshChild->SetVisibility(true);
			}
		}
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

// Called every frame
void ADayOneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ADayOneCharacter::RotateInPlace(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon) GetCharacterMovement()->bOrientRotationToMovement = false;
	if (Combat && Combat->EquippedWeapon) bUseControllerRotationYaw = true;
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.1f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

// Called to bind functionality to input
void ADayOneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
}

ECombatState ADayOneCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}
