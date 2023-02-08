// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DayOne/Interfaces/InteractWithCrosshairsInterface.h"
#include "GameFramework/Character.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "DayOne/Components/CombatState.h"
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

	void SetOverlappingWeapon(class AWeapon* Weapon);
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
	void PlayReloadMontage();
	void PlayElimMontage();

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	void UpdateHUDHealth();

	// character death
	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	
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
	UFUNCTION(BlueprintCallable, Category=Input)
	void ReloadButtonPressed();
	UFUNCTION(BlueprintCallable, Category=Input)
	void ReloadButtonReleased();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UWidgetComponent* OverheadWidget;

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();

	/**
	* Animation montages
	*/
	void PlayHitReactMontage();
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void OnPlayerStateInitialized();
	
	// Poll for any relelvant classes and initialize our HUD
	void PollInit();
	
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
	float TurnThreshold = 0.25f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/**
	* Player health
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY()
	class ADayOnePlayerController* DayOnePlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 1.5f;

	void ElimTimerFinished();

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

	UPROPERTY()
	class ADayOnePlayerState* DayOnePlayerState;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	ECombatState GetCombatState() const;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCharCamera;
};
