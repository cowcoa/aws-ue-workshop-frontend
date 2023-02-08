// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DayOne/Weapons/WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class DAYONE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	void ShowPickupWidget(bool bShowWidget);

	void SetWeaponState(EWeaponState State);

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	virtual void Fire(const FVector& HitTarget);

	virtual void Dropped();

	void SetHUDAmmo();

	bool IsEmpty();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();

	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	/** 
    * Zoomed FOV while aiming
    */

    UPROPERTY(EditAnywhere)
    float ZoomedFOV = 30.f;

    UPROPERTY(EditAnywhere)
    float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity = 30;

	// The number of unprocessed server requests for Ammo.
	// Incremented in SpendRound, decremented in ClientUpdateAmmo.
	int32 Sequence = 0;

	UPROPERTY()
	class ADayOneCharacter* DayOneOwnerCharacter;
	UPROPERTY()
	class ADayOnePlayerController* DayOneOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	* Textures for the weapon crosshairs
	*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	
	/** 
	* Automatic fire
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
};
