// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "BiasedPrivatePCH.h"
#include "BiasedBPLibrary.h"

DEFINE_LOG_CATEGORY(BiasedLog);

UBiasedBPLibrary::UBiasedBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

bool UBiasedBPLibrary::GenerateBiasedDieData(const TArray<FDieFace>& Faces, FBiasedDieData& OutBiasedDieData)
{

	TArray<FDieFace> Large, Small;
	int32 ProbabilityScale = Faces.Num();

	float TotalProbabilityCheck = 0.0;

	for (FDieFace Face : Faces)
	{
		if ( ( TotalProbabilityCheck += Face.Probability ) > 1.0f)
		{
			UE_LOG(BiasedLog, Warning, TEXT("Combined face probability is greater than 1."));
			return false;
		}

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

	if (TotalProbabilityCheck < 1.0f)
	{
		UE_LOG(BiasedLog, Warning, TEXT("Combined face probability is less than 1."));
		return false;
	}

	FBiasedDieData Data;

	bool bFinished = false;
	while (!bFinished)
	{
		if (Small.Num() > 0)
		{
			if (Large.Num() > 0)
			{
				FDieFace LargeFace = Large.Pop();
				FDieFace SmallFace = Small.Pop();

				//TPair<FDieFace, int32> ProbabilityPair = TPair<FDieFace, int32>(TPairInitializer<FDieFace, int32>(SmallFace, LargeFace.Value));
				
				OutBiasedDieData.AddDieFaceAlias(SmallFace, LargeFace.Value);

				LargeFace.Probability = (LargeFace.Probability + SmallFace.Probability) - 1;

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
				FDieFace Face = Small.Pop();
				Face.Probability = 1.0f;
				//TPair<FDieFace, int32> ProbabilityPair = TPair<FDieFace, int32>(TPairInitializer<FDieFace, int32>(Face, 0));
				//OutBiasedDieData.ProbablilityPairs.Push(ProbabilityPair);
				OutBiasedDieData.AddDieFaceAlias(Face, 0);
			}
		}
		else
		{
			FDieFace Face = Large.Pop();
			Face.Probability = 1.0f;
			//TPair<FDieFace, int32> ProbabilityPair = TPair<FDieFace, int32>(TPairInitializer<FDieFace, int32>(Face, 0));
			//OutBiasedDieData.ProbablilityPairs.Push(ProbabilityPair);
			OutBiasedDieData.AddDieFaceAlias(Face, 0);
		}

		if (Large.Num() == 0 && Small.Num() == 0)
		{
			bFinished = true;
		}
	}


	return true;
}

int32 UBiasedBPLibrary::RollBiasedDie(const FBiasedDieData& BiasedDieData)
{
	int32 RandIndex = FMath::Rand() % BiasedDieData.NumFaces();
	float RandRoll = FMath::FRand();

	return InternalRollBiasedDice(BiasedDieData, RandIndex, RandRoll);

}

int32 UBiasedBPLibrary::RollBiasedDieFromStream(const FBiasedDieData& BiasedDieData, const FRandomStream& RandomStream)
{
	int32 RandIndex = RandomStream.RandRange(0, BiasedDieData.NumFaces() - 1);
	float RandRoll = RandomStream.FRand();

	return InternalRollBiasedDice(BiasedDieData, RandIndex, RandRoll);
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

