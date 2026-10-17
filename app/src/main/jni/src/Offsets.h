#pragma once
#include <cstdint>
#include <string>

struct Offsets {
    uintptr_t GameFacade_c;
    uintptr_t Player_c;
    uintptr_t HitEvent;
    uintptr_t RealDamage;
    uintptr_t AimTarget;
    uintptr_t EndPos;
    uintptr_t ColliderType;
    uintptr_t Matrix;
    uintptr_t AimInfo;
    uintptr_t StartPos;
    uintptr_t OrigStartPos;
    uintptr_t RayDir;
    uintptr_t HitCollider;
    uintptr_t Transform;
    uintptr_t Transform2;
    uintptr_t MatrixPtr;
    uintptr_t Rotation;
    uintptr_t Position;
    uintptr_t Scale;
    uintptr_t FollowCamera;
    uintptr_t Camera;
    uintptr_t intPtrCam;
    uintptr_t StaticClass;
    uintptr_t CurrentGame;
    uintptr_t uiInGameScene;
    uintptr_t bigMapCtrl;
    uintptr_t mapContentCtrl;
    uintptr_t localMapMarkController;
    uintptr_t markPosition;
    uintptr_t localPlayer;
    uintptr_t GhostHack;
    uintptr_t m_Match;
    uintptr_t MatchRun;
    uintptr_t dictionary;
    uintptr_t avatarManager;
    uintptr_t umaAvatarSimple;
    uintptr_t TransformType;
    uintptr_t isVisible;
    uintptr_t umaData;
    uintptr_t ShadowState;
    uintptr_t XPose;
    uintptr_t Attributes;
    uintptr_t DamageCont;
    uintptr_t Weaponv1;
    uintptr_t AmmoLeft;
    uintptr_t ForceSpeed;
    uintptr_t FireScale;
    uintptr_t NoReload;
    uintptr_t PlayerData;
    uintptr_t PlayerName;
    uintptr_t Health;
    uintptr_t isDead;
    uintptr_t isTeam;
    uintptr_t isClientBot;
    uintptr_t HeadBone;
    uintptr_t HipBone;
    uintptr_t RootBone;
    uintptr_t Weapon;
    uintptr_t WeaponAtri;
    uintptr_t NoRecoil;
    uintptr_t AimLock;
    uintptr_t ColliderHead;
    uintptr_t ColliderBody;
    uintptr_t TimeService;
    uintptr_t FixedDelta;
    uintptr_t isFiring;
    uintptr_t isSight;
    uintptr_t AimRotation;
    uintptr_t AimRotationAux;
    uintptr_t inventoryManager;
    uintptr_t isCombined;
    uintptr_t itemOnHand;
    uintptr_t combinedWeapon;
    uintptr_t itemData;
    uintptr_t shortId;
    uintptr_t UserControl;
    uintptr_t IsHighFPS;
};

static const Offsets FFMax = {
        .GameFacade_c = 0xAD6DC18,
        .Player_c = 0xAD74BB0,
        .HitEvent = 0x98,
        .RealDamage = 0x40,
        .AimTarget = 0xE70,
        .EndPos = 0x74,
        .ColliderType = 0x44,
        .Matrix = 0x100,
        .AimInfo = 0xE58,
        .StartPos = 0x4C,
        .OrigStartPos = 0x74,
        .RayDir = 0x40,
        .HitCollider = 0x20,
        .Transform = 0x10,
        .Transform2 = 0x10,
        .MatrixPtr = 0x38,
        .Rotation = 0xA0,
        .Position = 0x90,
        .Scale = 0xAC,
        .FollowCamera = 0x698,
        .Camera = 0x30,
        .intPtrCam = 0x10,
        .StaticClass = 0xB8,
        .CurrentGame = 0x0,
        .uiInGameScene = 0x10,
        .bigMapCtrl = 0x458,
        .mapContentCtrl = 0xA8,
        .localMapMarkController = 0xF8,
        .markPosition = 0xB0,
        .localPlayer = 0xD8,
        .GhostHack = 0x814,
        .m_Match = 0x90,
        .MatchRun = 0xCC,
        .dictionary = 0xC0,
        .avatarManager = 0x778,
        .umaAvatarSimple = 0x138,
        .TransformType = 0x1294,
        .isVisible = 0x101,
        .umaData = 0x28,
        .ShadowState = 0x24C0,
        .XPose = 0x80,
        .Attributes = 0x770,
        .DamageCont = 0x318,
        .Weaponv1 = 0x38,
        .AmmoLeft = 0x610,
        .ForceSpeed = 0x3C8,
        .FireScale = 0x270,
        .NoReload = 0x111,
        .PlayerData = 0x70,
        .PlayerName = 0x498,
        .Health = 0x18,
        .isDead = 0x7C,
        .isTeam = 0x81,
        .isClientBot = 0x4A8,
        .HeadBone = 0x6A8, //0x640
        .HipBone = 0x6B0,
        .RootBone = 0x6D0,
        .Weapon = 0x608,
        .WeaponAtri = 0x80,
        .NoRecoil = 0x18,
        .AimLock = 0x644,
        .ColliderHead = 0x740,
        .ColliderBody = 0x80,
        .TimeService = 0x20,
        .FixedDelta = 0x2C,
        .isFiring = 0x850,
        .isSight = 0x8A0,
        .AimRotation = 0x61C,
        .AimRotationAux = 0x630,
        .inventoryManager = 0x748,
        .isCombined = 0x148,
        .itemOnHand = 0xA0,
        .combinedWeapon = 0xA8,
        .itemData = 0x28,
        .shortId = 0x40,
        .UserControl = 0x4D8,
        .IsHighFPS = 0x8924DA0
};

static const Offsets FFMaxInd = {
        .GameFacade_c = 0xAD68E18,
        .Player_c = 0xAD6FDA8,
        .HitEvent = 0x98,
        .RealDamage = 0x40,
        .AimTarget = 0xE70,
        .EndPos = 0x74,
        .ColliderType = 0x44,
        .Matrix = 0x100,
        .AimInfo = 0xE58,
        .StartPos = 0x4C,
        .OrigStartPos = 0x74,
        .RayDir = 0x40,
        .HitCollider = 0x20,
        .Transform = 0x10,
        .Transform2 = 0x10,
        .MatrixPtr = 0x38,
        .Rotation = 0xA0,
        .Position = 0x90,
        .Scale = 0xAC,
        .FollowCamera = 0x698,
        .Camera = 0x30,
        .intPtrCam = 0x10,
        .StaticClass = 0xB8,
        .CurrentGame = 0x0,
        .uiInGameScene = 0x10,
        .bigMapCtrl = 0x458,
        .mapContentCtrl = 0xA8,
        .localMapMarkController = 0xF8,
        .markPosition = 0xB0,
        .localPlayer = 0xD8,
        .GhostHack = 0x814,
        .m_Match = 0x90,
        .MatchRun = 0xCC,
        .dictionary = 0xC0,
        .avatarManager = 0x778,
        .umaAvatarSimple = 0x138,
        .TransformType = 0x1294,
        .isVisible = 0x101,
        .umaData = 0x28,
        .ShadowState = 0x24C0,
        .XPose = 0x80,
        .Attributes = 0x770,
        .DamageCont = 0x318,
        .Weaponv1 = 0x38,
        .AmmoLeft = 0x610,
        .ForceSpeed = 0x3C8,
        .FireScale = 0x270,
        .NoReload = 0x111,
        .PlayerData = 0x70,
        .PlayerName = 0x498,
        .Health = 0x18,
        .isDead = 0x7C,
        .isTeam = 0x81,
        .isClientBot = 0x4A8,
        .HeadBone = 0x6A8, //0x640
        .HipBone = 0x6B0,
        .RootBone = 0x6D0,
        .Weapon = 0x608,
        .WeaponAtri = 0x80,
        .NoRecoil = 0x18,
        .AimLock = 0x644,
        .ColliderHead = 0x740,
        .ColliderBody = 0x80,
        .TimeService = 0x20,
        .FixedDelta = 0x2C,
        .isFiring = 0x850,
        .isSight = 0x8A0,
        .AimRotation = 0x61C,
        .AimRotationAux = 0x630,
        .inventoryManager = 0x748,
        .isCombined = 0x148,
        .itemOnHand = 0xA0,
        .combinedWeapon = 0xA8,
        .itemData = 0x28,
        .shortId = 0x40,
        .UserControl = 0x4D8,
        .IsHighFPS = 0x8907638
};

extern const Offsets* currentOffsets;
void setCurrentVersion(const std::string& version);