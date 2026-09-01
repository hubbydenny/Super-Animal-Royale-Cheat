#pragma once

#include "../Math/Vectors.hpp"
#include <Windows.h>
#include <list>
#include <mutex>
#include <vector>
#include <cstdint>
#include <cstring>

// Generated using ReClass.NET + updated via dump.cs

enum struct EPlayerActions // GEnum3
{
	None,
	Healing,
	HealingCupgrade,
	Zipline
};

enum struct EPlatformType // GEnum26
{
	Anonymous,
	Steam,
	Xbox,
	PSN,
	Switch,
	FB
};

enum struct EWalkMode : uint8_t // GEnum34
{
	Slow,
	Normal,
	Roll,
	RollCreep,
	Vehicle,
	Downed,
	BananaStun,
	ParachuteFreefallSplat
};

// Generic IL2CPP Array layout in x64
template <typename T>
struct Il2CppArray
{
	char pad_0000[24];   //0x0000 - Object header (16 bytes) + bounds ptr (8 bytes)
	uint64_t max_length; //0x0018 - Array length
	T items[1];          //0x0020 - Elements start
};

// Anti-Cheat Toolkit ObscuredFloat struct layout (TypeDefIndex: 9365)
struct ACTkObscuredFloat
{
	int32_t currentCryptoKey; //0x0000 - XOR encryption key
	int32_t hiddenValue;      //0x0004 - Encrypted integer value
	int32_t hiddenValueOld;   //0x0008
	bool    inited;           //0x000C
	char    pad_000D[3];
	float   fakeValue;        //0x0010
	bool    fakeValueActive;  //0x0014
	char    pad_0015[3];
}; // Size: 0x0018 (24 bytes)

inline float ReadObscuredFloat(const ACTkObscuredFloat& obf)
{
	int32_t plain = obf.hiddenValue ^ obf.currentCryptoKey;
	float result = 0.0f;
	memcpy(&result, &plain, sizeof(float));
	return result;
}

inline void WriteObscuredFloat(ACTkObscuredFloat& obf, float newValue)
{
	int32_t plain;
	memcpy(&plain, &newValue, sizeof(float));
	obf.hiddenValue = plain ^ obf.currentCryptoKey;
	obf.fakeValue = newValue;
}

class LocalPlayerScript
{
public:
	char pad_0000[24]; //0x0000
	void* camera; //0x0018
	float horizInput; //0x0020
	float vertInput; //0x0024
	Vector2 velocity; //0x0028
	char pad_0030[8]; //0x0030
	void* gameLevelType; //0x0038
	void* mainGameScript; //0x0040
	class NetworkPlayer* player; //0x0048
	char pad_0050[116]; //0x0050
	bool canFlightEject; //0x00C4
	char pad_00C5[3]; //0x00C5
	Vector2 lastServerPosition; //0x00C8
	Vector2 lastServerPositionForBananaStun; //0x00D0
}; //Size: 0x00D8

class Bone
{
public:
	char pad_0000[16];   // 0x00
	void* data;          // 0x10 - BoneData
	void* skeleton;      // 0x18 - Skeleton
	class Bone* parent;  // 0x20 - Parent bone
	void* children;      // 0x28 - ExposedList<Bone>
	float x;             // 0x30
	float y;             // 0x34
	float rotation;      // 0x38
	float scaleX;        // 0x3C
	float scaleY;        // 0x40
	float shearX;        // 0x44
	float shearY;        // 0x48
	float ax;            // 0x4C
	float ay;            // 0x50
	float arotation;     // 0x54
	float ascaleX;       // 0x58
	float ascaleY;       // 0x5C
	float ashearX;       // 0x60
	float ashearY;       // 0x64
	float a;             // 0x68
	float b;             // 0x6C
	float worldX;        // 0x70
	float c;             // 0x74
	float d;             // 0x78
	float worldY;        // 0x7C
};

class NetworkPlayer
{
public:
	char pad_0000[24]; //0x0000 - MonoBehaviour base
	void* audioPlayerLoop; //0x0018
	void* audioPlayerIceSlide; //0x0020
	float volumeMultForHandlingLocalPlayerInAir; //0x0028
	char pad_002C[4]; //0x002C
	void* quickChatSounds; //0x0030
	char pad_0038[8]; //0x0038
	void* healingParticleObj; //0x0040
	char pad_0048[8]; //0x0048
	void* attachmentParticleObject; //0x0050
	char pad_0058[8]; //0x0058
	void* zombieChargedParticleSystem; //0x0060
	void* bananafiedParticleObject; //0x0068
	void* bossSpeedBoostSpriteObj; //0x0070
	char pad_0078[8]; //0x0078
	class GameCamera* gameCamera; //0x0080
	void* skeletonAnimation; //0x0088
	void* skeleton; //0x0090
	void* splashManager; //0x0098
	void* raceFollowerItem; //0x00A0
	void* aiPath; //0x00A8
	char pad_00B0[8]; //0x00B0
	class LocalPlayerScript* localPlayerScript; //0x00B8
	char pad_00C0[16]; //0x00C0
	int16_t playerID; //0x00D0
	char pad_00D2[6]; //0x00D2
	void* playfabID; //0x00D8 - string (IL2CPP)
	void* platformUserID; //0x00E0 - string
	int32_t platformType; //0x00E8
	void* playerName; //0x00F0 - IL2CPP string
	void* playerNameForHideNames; //0x00F8
	void* playerNameUppercase; //0x0100
	void* playerNameForHideNamesUppercase; //0x0108
	int32_t accountLevelIndex; //0x0110
	bool isAdminStartingGhost; //0x0114
	bool isDev; //0x0115
	bool isMod; //0x0116
	bool isFounder; //0x0117
	char pad_0118[64]; //0x0118 - charType..voiceType
	float currentGunAngleToMouse; //0x0158
	bool isPressingMoveKeys; //0x015C
	char pad_015D[3]; //0x015D
	float playerHP; //0x0160
	float playerHPMax; //0x0164
	bool playerIsDead; //0x0168
	char pad_0169[15]; //0x0169
	int32_t numKills; //0x0178
	int16_t killedByPlayerID; //0x017C
	char pad_017E[82]; //0x017E
	Il2CppArray<int16_t>* equipmentIDs; //0x01D0
	char pad_01D8[8]; //0x01D8
	uint8_t currentEquipmentIndex; //0x01E0
	uint8_t previousEquipmentIndex; //0x01E1
	char pad_01E2[46]; //0x01E2
	float healingJuice; //0x0210
	bool healingCurrently; //0x0214
	char pad_0215[11]; //0x0215
	uint8_t ductTapes; //0x0220
	bool ductTapingCurrently; //0x0221
	char pad_0222[22]; //0x0222
	uint8_t currArmorLvl; //0x0238
	char pad_0239[3]; //0x0239
	int32_t currArmorAmount; //0x023C
	int32_t grenadeCount; //0x0240
	ACTkObscuredFloat lastAttackTime; //0x0244 - ObscuredFloat (24 bytes)
	float lastAttackTimeMelee; //0x025C
	bool attacking; //0x0260
	bool currentAttackIsMelee; //0x0261
	bool currentAttackIsAuto; //0x0262
	bool windingUpAttack; //0x0263
	bool windingDownAttack; //0x0264
	bool currentAttackIsBugNet; //0x0265
	bool currentAttackIsMountSummon; //0x0266
	char pad_0267[185]; //0x0267
	Vector2 currentPosition; //0x0320
	float timeUntilChatBubbleIsRemoved; //0x0328
	Vector2 previousPosition; //0x032C
	char pad_0334[25]; //0x0334
	bool isParachuting; //0x034D
	char pad_034E[2]; //0x034E
	float currParachuteHeight; //0x0350
	bool isParachuteDiving; //0x0354
	char pad_0355[27]; //0x0355
	bool inFlight; //0x0370
	char pad_0371[12]; //0x0371
	EWalkMode currentWalkMode; //0x037D
	char pad_037E[72]; //0x037E
	bool isZombie; //0x03C6
	bool isJuicedUpZombie; //0x03C7
	float zombieJuiceMeter; //0x03C8
	char pad_03CC[72]; //0x03CC
	float rollStartTime; //0x0414
	float rollEndTime; //0x0418
	float bananaStunStartTime; //0x041C
	float bananaStunEndTime; //0x0420
	char pad_0424[24]; //0x0424
	bool didHitBunnyHopRoll; //0x043C
	char pad_043D[3]; //0x043D
	float bunnyHopCurrentBonus; //0x0440
	char pad_0444[84]; //0x0444
	class Bone* boneGun; //0x0498
	class Bone* boneGunLow; //0x04A0
	class Bone* boneGunLowAiming; //0x04A8
	char pad_04B0[165]; //0x04B0
	bool isWithinVisionBounds; //0x0555
}; //Size: 0x0556

// Safe weapon slot & ID helpers
inline int16_t GetActiveEquipmentID(const NetworkPlayer* p)
{
	if (!p || IsBadReadPtr(p, sizeof(NetworkPlayer))) return -1;
	if (!p->equipmentIDs || IsBadReadPtr(p->equipmentIDs, sizeof(Il2CppArray<int16_t>))) return -1;
	uint8_t slot = p->currentEquipmentIndex;
	if (slot >= p->equipmentIDs->max_length) return -1;
	return p->equipmentIDs->items[slot];
}

inline uint8_t GetActiveSlotIndex(const NetworkPlayer* p)
{
	if (!p || IsBadReadPtr(p, sizeof(NetworkPlayer))) return 255;
	return p->currentEquipmentIndex;
}

inline bool PlayerIsHoldingGun(const NetworkPlayer* p)
{
	if (!p) return false;
	uint8_t slot = GetActiveSlotIndex(p);
	return (slot == 0 || slot == 1);
}

inline bool PlayerIsHoldingMelee(const NetworkPlayer* p)
{
	if (!p) return false;
	uint8_t slot = GetActiveSlotIndex(p);
	return (slot == 2);
}

class GameCamera
{
public:
	char pad_0000[24]; //0x0000
	Vector2 camTarget; //0x0018
	float camAngleRadians; //0x0020
	float parachutingOffset; //0x0024
	char pad_0028[4]; //0x0028
	bool inFlight; //0x002C
	bool isIn32v32; //0x002D
	char pad_002E[2]; //0x002E
	float mainOrthoSize; //0x0030
	float parachuteOrthoSize; //0x0034
	float flightOrthoSize; //0x0038
	float ghostMaxOrthoSize; //0x003C
	float dampTime; //0x0040
	float inGasGreenFactor; //0x0044
	float creepZoomFactor; //0x0048
	bool isAdminGhost; //0x004C
	char pad_004D[19]; //0x004D
	bool adminGhostNeedsRefreshUI; //0x0060
	char pad_0061[47]; //0x0061
	void* unityCamera; //0x0090
}; //Size: 0x0098

class BoneData
{
public:
	char pad_0000[16]; //0x0000
	int32_t index; //0x0010
	char pad_0014[4]; //0x0014
	void* name; //0x0018 - string
	class BoneData* parent; //0x0020
	float length; //0x0028
	float x; //0x002C
	float y; //0x0030
	float rotation; //0x0034
	float scaleX; //0x0038
	float scaleY; //0x003C
}; //Size: 0x0040

class SkeletalBounds
{
public:
	char pad_0000[24]; //0x0000
	float minX; //0x0018
	float minY; //0x001C
	float maxX; //0x0020
	float maxY; //0x0024
}; //Size: 0x0028

constexpr const int LOCAL_PLAYER_INDEX = 0;
class GameContext
{
public:
	explicit GameContext()
		: localPlayer(nullptr)
	{}

	mutable std::mutex mtx;
	LocalPlayerScript* localPlayer = nullptr;

	std::list<NetworkPlayer*> players;
};
