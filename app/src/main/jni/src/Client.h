SocketClient client;

#define maxplayerCount 100

int startClient();
bool isConnected();
void stopClient();
bool initServer();
bool stopServer();

enum Mode {
    InitMode = 1,
    HackMode = 2,
    StopMode = 98,
    EspMode = 99
};

struct Request {
    int Mode;
    bool m_IsOn;
    int value;
    int ScreenWidth;
    int ScreenHeight;
};

enum class TransType : int {
    None = 0,
    Wukong = 1,
    Oreon = 2,
    Ball = 3,
    WukongSpecial = 4
};

struct PlayerData {
    Vector3 Head;
    Vector3 Toe;
    bool IsCaido;
    float Distancia;
    int Health;
    bool Name;
    char PlayerName[64];
    TransType SpecialAbilities;
    short WeaponId;
};

struct Response {
    bool Success;
    int PlayerCount;
    int EnemiesInFront;
    int EnemiesBehind;
    PlayerData Players[maxplayerCount];
};

std::string g_selectedVersion = "v2.130.x";

void SendVersionToServer(int versionIndex) {
    if (!isConnected()) return;
    Request request{100, false, versionIndex};
    client.send((void*)&request, sizeof(request));
}

int startClient(){
    client = SocketClient();
    if (!client.Create()) {
        return -1;
    }
    if (!client.Connect()) {
        return -1;
    }
    if (!initServer()) {
        return -1;
    }
    int index = (g_selectedVersion == "v2.133.x") ? 1 : 0;
    SendVersionToServer(index);
    return 0;
}

bool isConnected(){
    return client.connected;
}

void stopClient() {
    if(client.created && isConnected()){
        stopServer();
        client.Close();
    }
}

bool initServer() {
    Request request{Mode::InitMode, true, 0};
    int code = client.send((void*) &request, sizeof(request));
    if(code > 0) {
        Response response{};
        size_t length = client.receive((void*) &response);
        if(length > 0) {
            return response.Success;
        }
    }
    return false;
}

bool stopServer() {
    Request request{Mode::StopMode};
    int code = client.send((void*) &request, sizeof(request));
    if(code > 0) {
        Response response{};
        size_t length = client.receive((void*) &response);
        if(length > 0) {
            return response.Success;
        }
    }
    return false;
}

void SendBool(int32_t number, bool ftr) {
    Request request{number, ftr};
    int code = client.send((void*) &request, sizeof(request));
    if (code > 0) {
        Response response{};
        size_t length = client.receive((void*) &response);
        if (length > 0) {
            ;
        }
    }
}

void SendFloat(int32_t number, float very) {
    Request request{number, true, static_cast<int>(very)};
    float code = client.send((void*) &request, sizeof(request));
    if (code > 0) {
        Response response{};
        size_t length = client.receive((void*) &response);
        if (length > 0) {
            return;
        }
    }
}

Response getData(int screenWidth, int screenHeight){
    Request request{Mode::EspMode, true,0, screenWidth, screenHeight};
    int code = client.send((void*) &request, sizeof(request));
    if(code > 0){
        Response response{};
        size_t length = client.receive((void*) &response);
        if(length > 0){
            return response;
        }
    }
    Response response{false, 0};
    return response;
}



