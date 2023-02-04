// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DayOne/Interfaces/InteractWithCrosshairsInterface.h"
#include "GameFramework/Character.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "DayOneCharacter.generated.h"

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),

	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class DAYONE_API ADayOneCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADayOneCharacter();

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	FVector GetHitTarget() const;

	class UCameraComponent* GetFollowCamera() const;

	/**
	* Play montages
	*/
	void PlayFireMontage(bool bAiming);

	// Multicast play hit react montage
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHitReact();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	
	// Input process callback
	UFUNCTION(BlueprintCallable, Category=Input)
	void Move(const FVector2D& MoveValue);
	UFUNCTION(BlueprintCallable, Category=Input)
	void Look(const FVector2D& LookValue);
	UFUNCTION(BlueprintCallable, Category=Input)
	void JumpUp();
	UFUNCTION(BlueprintCallable, Category=Input)
	void EquipButtonPressed();
	UFUNCTION(BlueprintCallable, Category=Input)
	void CrouchButtonPressed();
	UFUNCTION(BlueprintCallable, Category=Input)
	void AimButtonPressed();
	UFUNCTION(BlueprintCallable, Category=Input)
	void AimButtonReleased();
	UFUNCTION(BlueprintCallable, Category=Input)
	void FireButtonPressed();
	UFUNCTION(BlueprintCallable, Category=Input)
	void FireButtonReleased();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UWidgetComponent* OverheadWidget;

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();

	/**
	* Animation montages
	*/
	void PlayHitReactMontage();
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;
	
private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	bool bRotateRootBone;
	float CalculateSpeed();

	// AO Data
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// turn
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCharCamera;
};
