#pragma once
#include <string>
#include <unordered_map>

class Namegun {
public:
    static void Init();
    static std::string GetGunName(short gunId);

private:
    static std::unordered_map<int, std::string> GunData;
};