#pragma once

#include "../Math/Vectors.hpp"
#include <list>
#include <mutex>
#include <vector>

// Generated using ReClass.NET

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
enum struct EWalkMode // GEnum34
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
}; //Size: 0x00D0

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
	char pad_0090[8]; //0x0090
	void* splashManager; //0x0098
	void* raceFollowerItem; //0x00A0
	void* aiPath; //0x00A8
	char pad_00B0[8]; //0x00B0
	class LocalPlayerScript* localPlayerScript; //0x00B8
	char pad_00C0[16]; //0x00C0 - two garbled class refs at 0xC0, 0xC8
	int16_t playerID; //0x00D0
	char pad_00D2[6]; //0x00D2
	void* playfabID; //0x00D8 - string (IL2CPP)
	void* platformUserID; //0x00E0 - string
	int32_t platformType; //0x00E8
	void* playerName; //0x00F0 - IL2CPP string (not wchar_t*)
	void* playerNameForHideNames; //0x00F8
	void* playerNameUppercase; //0x0100
	void* playerNameForHideNamesUppercase; //0x0108
	int32_t accountLevelIndex; //0x0110
	bool isAdminStartingGhost; //0x0114
	bool isDev; //0x0115
	bool isMod; //0x0116
	bool isFounder; //0x0117
	char pad_0118[64]; //0x0118 - charType..voiceType (0x118-0x157)
	float currentGunAngleToMouse; //0x0158
	bool isPressingMoveKeys; //0x015C
	char pad_015D[3]; //0x015D
	float playerHP; //0x0160
	float playerHPMax; //0x0164
	bool playerIsDead; //0x0168
	char pad_0169[15]; //0x0169
	int32_t numKills; //0x0178
	int16_t killedByPlayerID; //0x017C
	char pad_017E[146]; //0x017E - through equipment arrays etc
	float healingJuice; //0x0210
	bool healingCurrently; //0x0214
	char pad_0215[11]; //0x0215
	int8_t ductTapes; //0x0220
	char pad_0221[23]; //0x0221
	int8_t currArmorLvl; //0x0238
	char pad_0239[3]; //0x0239
	int32_t currArmorAmount; //0x023C
	int32_t grenadeCount; //0x0240
	char pad_0244[24]; //0x0244 - lastAttackTime (ObscuredFloat)
	float lastAttackTimeMelee; //0x025C
	bool attacking; //0x0260
	char pad_0261[191]; //0x0261 - attack fields, weapons, bones, etc
	Vector2 currentPosition; //0x0320 - current world position
	float timeUntilChatBubbleIsRemoved; //0x0328
	Vector2 previousPosition; //0x032C
	char pad_0334[60]; //0x0334 - footsteps, parachuting
	bool inFlight; //0x0370
	char pad_0371[85]; //0x0371 - flight, stun, walkmode, emotes
	bool isZombie; //0x03C6
	char pad_03C7[77]; //0x03C7
	float rollStartTime; //0x0414
	float rollEndTime; //0x0418
	char pad_041C[32]; //0x041C
	bool didHitBunnyHopRoll; //0x043C
	char pad_043D[3]; //0x043D
	float bunnyHopCurrentBonus; //0x0440
	char pad_0444[273]; //0x0444 - through bones, etc
	bool isWithinVisionBounds; //0x0555
}; //Size: 0x0556

class GameCamera
{
public:
	char pad_0000[24]; //0x0000
	Vector2 camTarget; //0x0018
	float camAngleRadians; //0x0020
	char pad_0024[8]; //0x0024
	bool inFlight; //0x002C
	char pad_002D[3]; //0x002D
	float mainOrthoSize; //0x0030
	char pad_0034[8]; //0x0034
	float ghostMaxOrthoSize; //0x003C
	char pad_0040[12]; //0x0040
	bool isAdminGhost; //0x004C
	char pad_004D[19]; //0x004D
	bool adminGhostNeedsRefreshUI; //0x0060
	char pad_0061[47]; //0x0061
	void* unityCamera; //0x0090
}; //Size: 0x0098

class Bone
{
public:
	char pad_0000[16]; //0x0000
	class BoneData* data; //0x0010
	char pad_0018[8]; //0x0018
	class Bone* parentBone; //0x0020
	char pad_0028[8]; //0x0028
	float x; //0x0030
	float y; //0x0034
	float rotation; //0x0038
	float scaleX; //0x003C
	float scaleY; //0x0040
	char pad_0044[8]; //0x0044
	float ax; //0x004C
	float ay; //0x0050
	float arotation; //0x0054
	float ascaleX; //0x0058
	float ascaleY; //0x005C
	char pad_0060[16]; //0x0060
	float worldX; //0x0070
	char pad_0074[8]; //0x0074
	float worldY; //0x007C
}; //Size: 0x0080

class BoneData
{
public:
	char pad_0000[16]; //0x0000
	int32_t index; //0x0010
	char pad_0014[4]; //0x0014
	wchar_t* name; //0x0018
	class BoneData* parent; //0x0020
	char pad_0028[4]; //0x0028
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