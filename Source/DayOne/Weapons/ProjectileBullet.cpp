// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "DayOne/Characters/DayOneCharacter.h"
#include "DayOne/PlayerController/DayOnePlayerController.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	ADayOneCharacter* OwnerCharacter = Cast<ADayOneCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ADayOnePlayerController* OwnerController = Cast<ADayOnePlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
