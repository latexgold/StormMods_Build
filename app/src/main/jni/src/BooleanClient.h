// Define enum types first
enum BoxType {
    BOX_STROKE = 0,
    BOX_FILLED = 1,
    BOX_CORNER = 2,
    BOX_ROUNDED = 3
};

enum LineType {
    LINE_TOP = 0,
    LINE_CENTER = 1,
    LINE_BOTTOM = 2
};

struct {
    bool aimhard = false;
    bool selectTarget = false;
    bool silen4a = false;
    float AimFov = 0;
    float AimDistance = 250;
    bool aimbotlock = false;
    bool aimlock = false;
    bool pullenemy = false;
    float ChestRate;
    float HeadRate;
    bool DiveKill = false;
    bool IgnoreKnock = false;
    bool SpeedTimer = false;
    bool ScaleDown = false;
    bool WallBug = false;
    bool SpinBot = false;
    bool GhostHack = false;
    bool KillInChest = false;
    bool TPMark = false;
    bool AimLag = false;
    bool PCMovement = false;
    bool FlyHack = false;
    bool TeleKill = false;
} pPlayer;

struct {
    bool activar = false;
} Actived;

struct {
    bool espLinha = false;
    bool espCaixa = false;
    bool espHealth = false;
    bool espDistance = false;
    bool espName = false;
    bool espSkeleton = false;
    bool espIden360 = false;
    bool drawFov = false;
    bool espCount = false;
    bool espWeapon = false;
    bool Info = true;
    float textSize = 15.0f;
    Color espColor = Color::White();
    BoxType boxType = BOX_STROKE;
    LineType lineType = LINE_TOP;
} pEspPlayer;

