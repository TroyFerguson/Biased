// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "BiasedPrivatePCH.h"
#include "BiasedBPLibrary.h"

DEFINE_LOG_CATEGORY(BiasedLog);

float UBiasedBPLibrary::FLOATING_POINT_TOLERANCE = 0.00001f;

const int32 UBiasedBPLibrary::ALIAS_NONE = 0;
const int32 UBiasedBPLibrary::INVALID_ROLL = 0;

UBiasedBPLibrary::UBiasedBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

bool UBiasedBPLibrary::GenerateBiasedDieData(const TArray<FDieFace>& Faces, FBiasedDieData& OutBiasedDieData)
{
	//Invalidate the die data before we process the new data
	OutBiasedDieData.Invalidate();

	TArray<FDieFace> Large, Small;
	int32 ProbabilityScale = Faces.Num();

	if (ProbabilityScale <= 0)
	{
		UE_LOG(BiasedLog, Warning, TEXT("Die has no faces."));
		return false;
	}

	float TotalProbabilityCheck = 0.0;

	//Sort faces into large and small faces based on their relative weightings
	for (FDieFace Face : Faces)
	{
		TotalProbabilityCheck += Face.Probability;

		FDieFace ScaledFace(Face.Value, Face.Probability * ProbabilityScale);

		if (ScaledFace.Probability >= 1.0f)
		{
			Large.Add(ScaledFace);
		}
		else
		{
			Small.Add(ScaledFace);
		}
	}

	//Quick check to see if the face data is normalised correctly
	if ( !FMath::IsNearlyEqual( TotalProbabilityCheck, 1.0f, FLOATING_POINT_TOLERANCE) )
	{
		UE_LOG(BiasedLog, Warning, TEXT("Combined face probability is not 1."));
		return false;
	}

	//FBiasedDieData Data;

	bool bFinished = false;
	while (!bFinished)
	{
		if (Small.Num() > 0)
		{
			if (Large.Num() > 0)
			{
				//Grab a large and a small face
				FDieFace LargeFace = Large.Pop();
				FDieFace SmallFace = Small.Pop();
				
				//Create an alias for the pair of chosen faces
				OutBiasedDieData.AddDieFaceAlias(SmallFace, LargeFace.Value);

				//Consume the amount of probability from the large face that we have used
				LargeFace.Probability = (LargeFace.Probability + SmallFace.Probability) - 1.0f;

				//push the excess back into the system to be used later
				if (LargeFace.Probability >= 1.0f)
				{
					Large.Add(LargeFace);
				}
				else
				{
					Small.Add(LargeFace);
				}
			}
			else
			{
				//We've run out of large faces so we just fully fill out the probability
				FDieFace Face = Small.Pop();
				Face.Probability = 1.0f;

				//We have no alias face here
				OutBiasedDieData.AddDieFaceAlias(Face, ALIAS_NONE);
			}
		}
		else
		{
			//We've run out of small faces so we just fully fill out the probability
			FDieFace Face = Large.Pop();
			Face.Probability = 1.0f;

			//We have no alias face here
			OutBiasedDieData.AddDieFaceAlias(Face, ALIAS_NONE);
		}

		if (Large.Num() == 0 && Small.Num() == 0)
		{
			bFinished = true;
		}
	}

	//Validate the die date so it is ready to be used
	OutBiasedDieData.Validate();
	return true;
}

int32 UBiasedBPLibrary::RollBiasedDie(const FBiasedDieData& BiasedDieData)
{
	if (!BiasedDieData.IsValid())
	{
		UE_LOG(BiasedLog, Warning, TEXT("Die data is invalid!"));
		return INVALID_ROLL;
	}

	int32 RandIndex = FMath::Rand() % BiasedDieData.NumFaces();
	float RandRoll = FMath::FRand();

	return InternalRollBiasedDice(BiasedDieData, RandIndex, RandRoll);

}

int32 UBiasedBPLibrary::RollBiasedDieFromStream(const FBiasedDieData& BiasedDieData, const FRandomStream& RandomStream)
{
	if (!BiasedDieData.IsValid())
	{
		UE_LOG(BiasedLog, Warning, TEXT("Die data is invalid!"));
		return INVALID_ROLL;
	}

	int32 RandIndex = RandomStream.RandRange(0, BiasedDieData.NumFaces() - 1);
	float RandRoll = RandomStream.FRand();

	return InternalRollBiasedDice(BiasedDieData, RandIndex, RandRoll);
}

void UBiasedBPLibrary::AdjustErrorCheckingTolerance(float NewTolerance)
{
	FLOATING_POINT_TOLERANCE = NewTolerance;
}

bool UBiasedBPLibrary::IsDieDataValid(const FBiasedDieData &BiasedDieData)
{
	return BiasedDieData.IsValid();
}

int32 UBiasedBPLibrary::InternalRollBiasedDice(const FBiasedDieData &BiasedDieData, int32 RandIndex, float RandRoll)
{
	FDieFaceAlias PairToRoll = BiasedDieData[RandIndex];

	int32 Result = PairToRoll.Key.Value;
	if (RandRoll > PairToRoll.Key.Probability)
	{
		Result = PairToRoll.Value;
	}

	return Result;
}