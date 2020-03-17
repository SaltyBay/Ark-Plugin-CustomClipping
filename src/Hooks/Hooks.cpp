/****************************************************************************
* Copyright (C) 2019 by Matthias Birnthaler                                 *
*                                                                           *
* This file is part of the OverwriteCuddleWalking Plugin for Ark Server API *
*                                                                           *
*   The OverwriteCuddleWalking Plugin is free software: you can             *
*	redistribute it and/or modify it under the terms of the GNU Lesser      *
*	General Public License as published by the Free Software Foundation,    *
*	either version 3 of the License, or (at your option) any later version. *
*                                                                           *
*   The OverwriteCuddleWalking Plugin is distributed in the hope that it    *
*   will be useful, but WITHOUT ANY WARRANTY; without even the implied      *
* 	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.        *
* 	See the GNU Lesser General Public License for more details.             *
*                                                                           *
****************************************************************************/

/**
* \file Hooks.cpp
* \author Matthias Birnthaler 
* \date 19 March 2019
* \brief File containing the implementation for all needed Hooks  
*
*/

#include "Hooks.h"



/**
* \var FoodList
* \brief list given food given by json config
*/
static std::vector<std::wstring> FoodList;  
static int  Hook_APrimalStructure_IsAllowedToBuild(APrimalStructure* _this, APlayerController* PC, FVector AtLocation, FRotator AtRotation, FPlacementData* OutPlacementData, bool bDontAdjustForMaxRange, FRotator PlayerViewRotation, bool bFinalPlacement);
static FString GetBlueprint(UObjectBase* object);


/**
* \brief GetBlueprint
*
* This function determines the blueprint of a UObjectBase
*
* \param[in] object UObjectBase the object to check
* \return FString the blueprint
*/
static FString GetBlueprint(UObjectBase* object)
{
	if (object != nullptr && object->ClassField() != nullptr)
	{
		FString path_name;
		object->ClassField()->GetDefaultObject(true)->GetFullName(&path_name, nullptr);

		if (int find_index = 0; path_name.FindChar(' ', find_index))
		{
			path_name = "Blueprint'" + path_name.Mid(find_index + 1,
				path_name.Len() - (find_index + (path_name.EndsWith(
					"_C", ESearchCase::
					CaseSensitive)
					? 3
					: 1))) + "'";
			return path_name.Replace(L"Default__", L"", ESearchCase::CaseSensitive);
		}
	}

	return FString("");
}



/**
* \brief Initialisation of needed Hooks 
*
* This function initialise all needed Hooks and read the relevant information from the json config file.
*
* \return void
*/
void InitHooks(void)
{
	ArkApi::GetHooks().SetHook("APrimalStructure.IsAllowedToBuild", 
		&Hook_APrimalStructure_IsAllowedToBuild, 
		&APrimalStructure_IsAllowedToBuild_original);

	ArkApi::GetHooks().SetHook("UActorChannel.ReplicateActor", 
		&Hook_UActorChannel_ReplicateActor,
		&UActorChannel_ReplicateActor_original);

}


/**
* \brief Cancellation of needed Hooks
*
* This function removes all needed Hooks.
*
* \return void
*/
void RemoveHooks(void)
{
	ArkApi::GetHooks().DisableHook("APrimalStructure.IsAllowedToBuild", 
		&Hook_APrimalStructure_IsAllowedToBuild);

	ArkApi::GetHooks().DisableHook("UActorChannel.ReplicateActor", 
		&Hook_UActorChannel_ReplicateActor);

}


/**
* \brief Hook of APrimalDinoCharacter_UpdateBabyCuddling
*
* This function validates if the \p NewBabyCuddleType is walking or food and calls the original UpdateBabyCuddling function with modified parameters.
* The parameter \p NewBabyCuddleType will modify to food and the \p NewBabyCuddleFood will be filled by one random selected food of the FoodList.
* In case the \p NewBabyCuddleType is not walking this function calls the original UpdateBabyCuddling function without any modified parameters.
* In case of an error the original UpdateBabyCuddling function without any modified parameters.
*
* \param[in] _this the dino that gets updated  
* \param[in] NewBabyNextCuddleTime the cuddle time (server time in seconds)
* \param[in] NewBabyCuddleType the cuddletype (cuddle/food/walking) that the dinos requests 
* \param[in] NewBabyCuddleFood the food that the dino requests (if cuddletype is food) 
* \warning global variable used 
* \return void
*/
int  Hook_APrimalStructure_IsAllowedToBuild(APrimalStructure* _this, APlayerController* PC, FVector AtLocation, FRotator AtRotation, 
	FPlacementData* OutPlacementData, bool bDontAdjustForMaxRange, FRotator PlayerViewRotation, bool bFinalPlacement)
{
	const auto res =  APrimalStructure_IsAllowedToBuild_original(_this, PC, AtLocation, AtRotation, OutPlacementData, bDontAdjustForMaxRange, PlayerViewRotation, bFinalPlacement);

	FString string; 


	string = GetBlueprint(_this);



	Log::GetLog()->error("IsAllowed: {}", string.ToString());
	Log::GetLog()->error("IsAllowed result: {}", res);


	return res;
}


struct UNetConnection
{
	TSharedPtr<FUniqueNetId, 0>& PlayerIdField()
	{
		return *GetNativePointerField<TSharedPtr<FUniqueNetId, 0>*>(this, "UNetConnection.PlayerId");
	}
};

struct UChannel
{
	UNetConnection* ConnectionField() { return *GetNativePointerField<UNetConnection **>(this, "UChannel.Connection"); }
};

struct UActorChannel : UChannel
{
};


bool Hook_UActorChannel_ReplicateActor(UActorChannel* _this)
{

	Log::GetLog()->error("ActorChannel called");

	if (_this->ConnectionField())
	{
		FUniqueNetIdSteam* steam_net_id = static_cast<FUniqueNetIdSteam*>(_this->ConnectionField()
			->PlayerIdField().Get());
		const uint64 steam_id = steam_net_id->UniqueNetId;

	}

	return UActorChannel_ReplicateActor_original(_this);
}