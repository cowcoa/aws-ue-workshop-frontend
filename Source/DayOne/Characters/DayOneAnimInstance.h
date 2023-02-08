// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DayOneCharacter.h"
#include "DayOneAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UDayOneAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category=Character)
	class ADayOneCharacter* DayOneCharacter;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bAiming;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float Lean;
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float AO_Pitch;

	// FAbrak
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	// rotate right hand
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bElimmed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK;
};
