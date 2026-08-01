#pragma once

namespace Debug
{
	static void Print(const FString& Msg, const FColor& Color = FColor::Cyan, int32 InKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, Color, Msg);

			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		}
	}
	
	static void Print(const FString& FloatTitle, float ValueToPrint, int32 InKey = -1, const FColor& Color = FColor::Cyan)
	{
		if (GEngine)
		{
			const FString FinalMsg = FloatTitle + TEXT(": ") + FString::SanitizeFloat(ValueToPrint);
			
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, Color, FinalMsg);
			
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FinalMsg);
		}
	}
}