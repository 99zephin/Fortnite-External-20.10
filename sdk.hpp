#pragma once
#include "render.hpp"
#include "gloabls.hpp"
#include "vector.hpp"
#include "driver.hpp"

//General Util
inline DWORD_PTR Uworld;
inline DWORD_PTR LocalPawn;
inline DWORD_PTR PlayerState;
inline DWORD_PTR Localplayer;
inline DWORD_PTR Rootcomp;
inline DWORD_PTR relativelocation;
inline DWORD_PTR PlayerController;
inline DWORD_PTR Persistentlevel;
inline DWORD_PTR Ulevel;

//Other Util
inline int localplayerID;
inline uintptr_t curactoridA;
inline uint64_t CurrentActor;
inline Vector3 Camera;
inline Vector3 localactorpos;

//World To Screen util
inline float FovAngle;

typedef struct _FNlEntity {
	uint64_t Actor;
	int ID;
	uint64_t mesh;
}FNlEntity;
std::vector<FNlEntity> entityList;

__forceinline static std::string ReadGetNameFromFName(int key) {
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(sdk::module_base + 0xC42D200 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset);
	uint16_t nameEntry = read<uint16_t>(NamePoolChunk);

	int nameLength = nameEntry >> 6;
	char buff[1024];
	if ((uint32_t)nameLength) {
		char* v2 = buff; // rbx
		unsigned int v4 = nameLength;
		unsigned int v5; // eax
		__int64 result; // rax
		int v7; // ecx
		unsigned int v8; // kr00_4
		__int64 v9; // ecx

		readwtf(NamePoolChunk + 4, buff, v4);

		v5 = 0;
		result = 28i64;
		if (v4) {
			do {
				++v2;
				v7 = v5++ | 0xB000;
				v8 = v7 + result;
				BYTE(v7) = v8 ^ ~*(BYTE*)(v2 - 1);
				result = v8 >> 2;
				*(BYTE*)(v2 - 1) = v7;
			} while (v5 < v4);
		}
		buff[nameLength] = '\0';
		return std::string(buff);
	}
	else {
		return "";
	}
}

__forceinline static std::string GetNameFromFName(int key) {
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(sdk::module_base + 0xC42D200 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	if (read<uint16_t>(NamePoolChunk) < 64) {
		auto a1 = read<DWORD>(NamePoolChunk + 4);
		return ReadGetNameFromFName(a1);
	}
	else {
		return ReadGetNameFromFName(key);
	}
}

__forceinline FTransform GetBoneIndex(DWORD_PTR mesh, int index) {
	DWORD_PTR bonearray;
	bonearray = read<uintptr_t>(mesh + 0x590); // 0x5A0
	if (bonearray == NULL)
		bonearray = read<uintptr_t>(mesh + 0x590);

	return read<FTransform>(bonearray + (index * 0x60));
}

__forceinline Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id) {
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = read<FTransform>(mesh + 0x240);
	D3DMATRIX Matrix;

	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

Vector3 ProjectWorldToScreen(Vector3 WorldLocation) {
	Vector3 Screenlocation = Vector3(0, 0, 0);
	auto chain69 = read<uintptr_t>(Localplayer + 0xC8);
	uint64_t chain699 = read<uintptr_t>(chain69 + 8);
	Camera.x = read<double>(chain699 + 0xAE0);
	Camera.y = read<double>(Rootcomp + 0x148);
	double test = asin(Camera.x);
	double degrees = test * (180.0 / M_PI);
	Camera.x = degrees;
	if (Camera.y < 0)
		Camera.y = 360 + Camera.y;
	D3DMATRIX tempMatrix = Matrix(Camera);
	Vector3 vAxisX, vAxisY, vAxisZ;
	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	uint64_t chain = read<uint64_t>(Localplayer + 0x70);
	uint64_t chain1 = read<uint64_t>(chain + 0x98);
	uint64_t chain2 = read<uint64_t>(chain1 + 0x180);

	Vector3 vDelta = WorldLocation - read<Vector3>(chain2 + 0x20);
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));
	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	double zoom = read<double>(chain699 + 0x690);
	FovAngle = 80.0f / (zoom / 1.19f);
	double ScreenCenterX = Width / 2;
	double ScreenCenterY = Height / 2;
	double ScreenCenterZ = Height / 2;
	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.z = ScreenCenterZ - vTransformed.z * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	return Screenlocation;
}

__forceinline double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

__forceinline void SetMouseAbsPosition(DWORD x, DWORD y) {
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx = x;
	input.mi.dy = y;
	SendInput(1, &input, sizeof(input));
}

__forceinline void call_aim(double x, double y) {
	double center_x = (ImGui::GetIO().DisplaySize.x / 2);
	double center_y = (ImGui::GetIO().DisplaySize.y / 2);
	double target_x = 0;
	double target_y = 0;

	if (x != 0.f) {
		if (x > center_x) {
			target_x = -(center_x - x);
			target_x /= config::aimbot::smoothing;
			if (target_x + center_x > center_x * 2.f) target_x = 0.f;
		}

		if (x < center_x) {
			target_x = x - center_x;
			target_x /= config::aimbot::smoothing;
			if (target_x + center_x < 0.f) target_x = 0.f;
		}
	}

	if (y != 0.f) {
		if (y > center_y) {
			target_y = -(center_y - y);
			target_y /= config::aimbot::smoothing;
			if (target_y + center_y > center_y * 2.f) target_y = 0.f;
		}

		if (y < center_y) {
			target_y = y - center_y;
			target_y /= config::aimbot::smoothing;
			if (target_y + center_y < 0.f) target_y = 0.f;
		}
	}

	float theNum = floor(target_x / config::aimbot::smoothing);
	float result = theNum / 6.666666666666667f;

	float theNum1 = floor(target_y / config::aimbot::smoothing);
	float resulte = theNum1 / 6.666666666666667f;
	float result1 = -(resulte);

	SetMouseAbsPosition(static_cast<DWORD>(target_x), static_cast<DWORD>(target_y));
}