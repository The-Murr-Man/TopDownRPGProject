// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Input/RPGInputConfig.h"
#include "RPGInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const URPGInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);

};

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
inline void URPGInputComponent::BindAbilityActions(const URPGInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	// Loops throug Ability Input Actions
	for (const FRPGInputAction& Action : InputConfig->AbilityInputActions)
	{
		// Checks if both InputAction and InputTag are valid
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			// If PressedFunc is called Bind the Input Action and pass the Input Tag to the callback
			if (PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}

			// If ReleasedFunc is called Bind the Input Action and pass the Input Tag to the callback
			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}

			// If HeldFunc is called Bind the Input Action and pass the Input Tag to the callback
			if (HeldFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
