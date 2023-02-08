// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOneAnimInstance.h"

#include "DayOne/Components/CombatState.h"
#include "DayOne/Weapons/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UDayOneAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	DayOneCharacter = Cast<ADayOneCharacter>(TryGetPawnOwner());
}

void UDayOneAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (DayOneCharacter == nullptr)
	{
		DayOneCharacter = Cast<ADayOneCharacter>(TryGetPawnOwner());
	}
	if (DayOneCharacter == nullptr) return;

	FVector Velocity = DayOneCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = DayOneCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = DayOneCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = DayOneCharacter->IsWeaponEquipped();
	EquippedWeapon = DayOneCharacter->GetEquippedWeapon();
	bIsCrouched = DayOneCharacter->bIsCrouched;
	bAiming = DayOneCharacter->IsAiming();
	TurningInPlace = DayOneCharacter->GetTurningInPlace();
	bRotateRootBone = DayOneCharacter->ShouldRotateRootBone();
	bElimmed = DayOneCharacter->IsElimmed();

	// Offset Yaw for Strafing
	FRotator AimRotation = DayOneCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(DayOneCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = DayOneCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = DayOneCharacter->GetAO_Yaw();
	AO_Pitch = DayOneCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && DayOneCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		DayOneCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		
		if (DayOneCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - DayOneCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);

			// debug
			const USkeletalMeshSocket* MuzzleFlashSocket = EquippedWeapon->GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
			FTransform MFSocketTransform = MuzzleFlashSocket->GetSocketTransform(EquippedWeapon->GetWeaponMesh());
			DrawDebugLine(GetWorld(), MFSocketTransform.GetLocation(), DayOneCharacter->GetHitTarget(), FColor::Red);
			const FVector MFDirection(FRotationMatrix(MFSocketTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
			DrawDebugLine(GetWorld(), MFSocketTransform.GetLocation(), MFSocketTransform.GetLocation() + MFDirection * 1000.0f, FColor::Orange);
		}
	}

	bUseFABRIK = DayOneCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
}
