#pragma once
#include "sdk.hpp"

__forceinline void setup_world() {
	while (true) {
		std::vector<FNlEntity> entList;

		Uworld = read<DWORD_PTR>(sdk::module_base + 0xC3EF7C8); //GWorld //0xc445278
		DWORD_PTR Gameinstance = read<DWORD_PTR>(Uworld + 0x190); //OwningGameInstance
		DWORD_PTR LocalPlayers = read<DWORD_PTR>(Gameinstance + 0x38); //LocalPlayers

		Localplayer = read<DWORD_PTR>(LocalPlayers);
		PlayerController = read<DWORD_PTR>(Localplayer + 0x30); //UPlayer	PlayerController

		LocalPawn = read<DWORD_PTR>(PlayerController + 0x320); //0x338

		PlayerState = read<DWORD_PTR>(LocalPawn + 0x2A0);  //APawn	PlayerState
		Rootcomp = read<DWORD_PTR>(LocalPawn + 0x188); //AActor	RootComponent

		relativelocation = read<DWORD_PTR>(Rootcomp + 0x128); //USceneComponent	RelativeLocation

		if (LocalPawn != 0) {
			localplayerID = read<int>(LocalPawn + 0x18);
		}

		Persistentlevel = read<DWORD_PTR>(Uworld + 0x30); //UWorld	PersistentLevel	0x30
		DWORD ActorCount = read<DWORD>(Persistentlevel + 0xA0);
		DWORD_PTR AActors = read<DWORD_PTR>(Persistentlevel + 0x98);

		for (int i = 0; i < ActorCount; ++i) {
			DWORD_PTR CurrentActor = read<DWORD_PTR>(AActors + i * 0x8);
			//if (CurrentActor = LocalPawn) continue;
			uint64_t CurrentActorMesh = read<uint64_t>(CurrentActor + 0x300); //0x318
			int curactorid = read<int>(CurrentActor + 0x18);
			std::string Names = GetNameFromFName(curactorid);
			DWORD64 actorPlayerState = read<DWORD64>(CurrentActor + 0x2A0);
			int MyTeamId = read<int>(PlayerState + 0x1020);
			int ActorTeamId = read<int>(actorPlayerState + 0x1020);
			if (MyTeamId == ActorTeamId) continue;

			//std::cout << Names << std::endl;
			if (Names.find(("PlayerPawn")) != std::string::npos || Names.find(("PlayerPawn_Athena_C")) != std::string::npos) {
				FNlEntity fnlEntity{ };
				fnlEntity.Actor = CurrentActor;
				fnlEntity.mesh = CurrentActorMesh;
				fnlEntity.ID = curactorid;
				entList.push_back(fnlEntity);
			}
		}
		entityList = entList;
		//Sleep(1);
	}
}

__forceinline void setup_player() {
	auto entityListCopy = entityList;
	float closestDistance = FLT_MAX;
	DWORD_PTR closestPawn = NULL;
	uint64_t actors = read<uint64_t>(Persistentlevel + 0xA0);
	DWORD_PTR AActors = read<DWORD_PTR>(Persistentlevel + 0x98);
	for (int i = 0; i < entityListCopy.size(); ++i) {
		FNlEntity entity = entityListCopy.at(i);
		uint64_t CurrentActor = read<uint64_t>(AActors + i * 0x8);
		uint64_t CurrentActorMesh = read<uint64_t>(CurrentActor + 0x300);

		localactorpos = read<Vector3>(Rootcomp + 0x128);

		Vector3 vBaseBone = GetBoneWithRotation(entity.mesh, 0);
		Vector3 vBaseBoneOut = ProjectWorldToScreen(vBaseBone);
		Vector3 vHeadBone = GetBoneWithRotation(entity.mesh, 68);
		Vector3 vHeadBoneOut = ProjectWorldToScreen(vHeadBone);

		float distance = localactorpos.Distance(vHeadBone) / 100.f;

		float BoxHeight = (float)(vHeadBoneOut.y - vBaseBoneOut.y);
		float BoxWidth = BoxHeight * 0.380f;
		float LeftX = (float)vHeadBoneOut.x - (BoxWidth / 1);
		float LeftY = (float)vBaseBoneOut.y;
		float CornerHeight = abs(vHeadBoneOut.y - vHeadBoneOut.y);
		float CornerWidth = CornerHeight * 0.75;

		auto balls = ImColor(255, 0, 0);

		if (config::visuals::enable) {
			if (config::visuals::snapline) {
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height), ImVec2(vBaseBoneOut.x, vBaseBoneOut.y), balls, 2.f);
			}
		}
	}
}