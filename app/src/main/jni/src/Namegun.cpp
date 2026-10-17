#include "Namegun.h"
#include <string>
#include <unordered_map>

std::unordered_map<int, std::string> Namegun::GunData;

void Namegun::Init() {
    if (!GunData.empty()) return;
    GunData.reserve(200);

    // SPECIALS
    GunData.emplace(6016, "CALL AIRDROP");
    GunData.emplace(10006, "TRUCK DROP");
    GunData.emplace(2102, "TURRET");
    GunData.emplace(-15524 + 25000, "HEALING MINI DRONE");
    GunData.emplace(-15521 + 25000, "HEALING MINI DRONE (1 USE)");
    GunData.emplace(21001, "HEALING PISTOL");
    GunData.emplace(-25534, "HAMMER");
    GunData.emplace(7812, "PIGGY BANK");

    // BOTS
    GunData.emplace(0, "AK47");

    // ASSAULT RIFLES
    GunData.emplace(2, "M4A1");
    GunData.emplace(80, "M4A1-I");
    GunData.emplace(81, "M4A1-II");
    GunData.emplace(82, "M4A1-III");
    GunData.emplace(7821, "M4A1 LV.99");
    GunData.emplace(6, "AK47");
    GunData.emplace(7802, "AK47 LV.99");
    GunData.emplace(11, "M14");
    GunData.emplace(63, "M14-I");
    GunData.emplace(126, "M14-II");
    GunData.emplace(127, "M14-III");
    GunData.emplace(12, "SCAR");
    GunData.emplace(178, "SCAR-I");
    GunData.emplace(179, "SCAR-II");
    GunData.emplace(180, "SCAR-III");
    GunData.emplace(7803, "SCAR LV.99");
    GunData.emplace(14, "GROZA");
    GunData.emplace(629, "GROZA");
    GunData.emplace(70, "GROZA-X");
    GunData.emplace(7859, "GROZA LV.99");
    GunData.emplace(24, "FAMAS");
    GunData.emplace(67, "FAMAS-I");
    GunData.emplace(130, "FAMAS-II");
    GunData.emplace(131, "FAMAS-III");
    GunData.emplace(635, "FAMAS LV.99");
    GunData.emplace(28, "XM8");
    GunData.emplace(33, "AN94");
    GunData.emplace(7805, "AN94 LV.99");
    GunData.emplace(39, "PLASMA");
    GunData.emplace(7806, "PLASMA LV.99");
    GunData.emplace(46, "AUG");
    GunData.emplace(193, "AUG-I");
    GunData.emplace(194, "AUG-II");
    GunData.emplace(195, "AUG-III");
    GunData.emplace(7807, "AUG LV.99");
    GunData.emplace(47, "PARAFAL");
    GunData.emplace(7808, "PARAFAL LV.99");
    GunData.emplace(57, "KING FISHER");
    GunData.emplace(7809, "KING FISHER LV.99");
    GunData.emplace(73, "G36-ASSAULT");
    GunData.emplace(74, "G36-RANGE");
    GunData.emplace(7810, "G36-ASSAULT LV.99");
    GunData.emplace(7811, "G36-RANGE LV.99");

    // MARKSMAN RIFLES
    GunData.emplace(18, "SKS");
    GunData.emplace(7817, "SKS LV.99");
    GunData.emplace(26, "SVD");
    GunData.emplace(72, "SVD-Y");
    GunData.emplace(48, "WOODPECKER");
    GunData.emplace(7818, "WOODPECKER LV.99");
    GunData.emplace(89, "AC80");
    GunData.emplace(7820, "AC80 LV.99");
    GunData.emplace(21006, "WINCHESTER");
    GunData.emplace(21007, "WINCHESTER-I");
    GunData.emplace(21008, "WINCHESTER-II");
    GunData.emplace(21009, "WINCHESTER-III");
    GunData.emplace(7801, "WINCHESTER LV.99");

    // MACHINE GUNS
    GunData.emplace(19, "M249");
    GunData.emplace(631, "M249");
    GunData.emplace(71, "M249-X");
    GunData.emplace(7866, "M249 LV.99");
    GunData.emplace(30, "M60");
    GunData.emplace(61, "M60-I");
    GunData.emplace(122, "M60-II");
    GunData.emplace(123, "M60-III");
    GunData.emplace(7813, "M60 LV.99");
    GunData.emplace(54, "KORD");
    GunData.emplace(7814, "KORD LV.99");

    // SMGS
    GunData.emplace(7, "UMP");
    GunData.emplace(7840, "UMP LV.99");
    GunData.emplace(8, "MP5");
    GunData.emplace(60, "MP5-I");
    GunData.emplace(120, "MP5-II");
    GunData.emplace(121, "MP5-III");
    GunData.emplace(7841, "MP5 LV.99");
    GunData.emplace(13, "VSS");
    GunData.emplace(62, "VSS-I");
    GunData.emplace(124, "VSS-II");
    GunData.emplace(125, "VSS-III");
    GunData.emplace(7842, "VSS LV.99");
    GunData.emplace(15, "MP40");
    GunData.emplace(7843, "MP40 LV.99");
    GunData.emplace(32, "P90");
    GunData.emplace(7844, "P90 LV.99");
    GunData.emplace(35, "CG15");
    GunData.emplace(43, "THOMPSON");
    GunData.emplace(637, "THOMPSON");
    GunData.emplace(21020, "THOMPSON-X");
    GunData.emplace(7863, "THOMPSON LV.99");
    GunData.emplace(49, "VECTOR");
    GunData.emplace(69, "DOUBLE VECTOR");
    GunData.emplace(88, "MAC10");
    GunData.emplace(228, "MAC10-I");
    GunData.emplace(229, "MAC10-II");
    GunData.emplace(230, "MAC10-III");
    GunData.emplace(7847, "MAC10 LV.99");
    GunData.emplace(150, "BIZON");
    GunData.emplace(7848, "BIZON LV.99");

    // SHOTGUNS
    GunData.emplace(5, "M1014");
    GunData.emplace(184, "M1014-I");
    GunData.emplace(185, "M1014-II");
    GunData.emplace(186, "M1014-III");
    GunData.emplace(7835, "M1014 LV.99");
    GunData.emplace(29, "SPAS12");
    GunData.emplace(41, "M1887");
    GunData.emplace(7862, "M1887 LV.99");
    GunData.emplace(119, "M1887-X");
    GunData.emplace(50, "MAG-7");
    GunData.emplace(7837, "MAG-7 LV.99");
    GunData.emplace(86, "CHARGE BUSTER");
    GunData.emplace(7855, "CHARGE BUSTER LV.99");
    GunData.emplace(181, "TROGON-SHOTGUN");
    GunData.emplace(182, "TROGON-GRENADE");
    GunData.emplace(21002, "M590");
    GunData.emplace(7839, "M590 LV.99");

    // SNIPER RIFLES
    GunData.emplace(4, "AWM");
    GunData.emplace(65, "AWM-Y");
    GunData.emplace(7858, "AWM LV.99");
    GunData.emplace(21, "KAR98K");
    GunData.emplace(64, "KAR98K-I");
    GunData.emplace(128, "KAR98K-II");
    GunData.emplace(129, "KAR98K-III");
    GunData.emplace(7825, "KAR98K LV.99");
    GunData.emplace(45, "M82B");
    GunData.emplace(7822, "M82B LV.99");
    GunData.emplace(75, "M24");
    GunData.emplace(805, "M24-I");
    GunData.emplace(806, "M24-II");
    GunData.emplace(807, "M24-III");
    GunData.emplace(7823, "M24 LV.99");
    GunData.emplace(78, "HEAL SNIPER");
    GunData.emplace(197, "VSK94");
    GunData.emplace(7824, "VSK94 LV.99");

    // PISTOLS
    GunData.emplace(3, "USP");
    GunData.emplace(56, "USP-2");
    GunData.emplace(9, "DESERT EAGLE");
    GunData.emplace(10, "G18");
    GunData.emplace(20, "M1873");
    GunData.emplace(25, "M500");
    GunData.emplace(55, "M1917");
    GunData.emplace(58, "MINI UZI");
    GunData.emplace(93, "HEALING PISTOL");

    // MELEE WEAPONS
    GunData.emplace(16, "PAN");
    GunData.emplace(17, "MACHETE");
    GunData.emplace(27, "BAT");
    GunData.emplace(34, "KATANA");
    GunData.emplace(51, "SCYTHE");
    GunData.emplace(53, "FF KNIFE");

    // FIST
    GunData.emplace(1, "FIST");

    // GRENADES
    GunData.emplace(601, "GRENADE");
    GunData.emplace(603, "SMOKE GRENADE");
    GunData.emplace(1201, "GLOO WALL");
    GunData.emplace(1204, "GLOO WALL");
    GunData.emplace(602, "FLASHBANG");
    GunData.emplace(608, "ICE BOMB");

    // OTHERS
    GunData.emplace(23, "M79");
    GunData.emplace(36, "RGS-50");
    GunData.emplace(196, "FGL-24");

    // MISC
    GunData.emplace(100, "FLAMETHROWER");
    GunData.emplace(99, "SHIELD GUN");
    GunData.emplace(617, "WALL BREAKER");
    GunData.emplace(1401, "LAND MINES");
    GunData.emplace(1006, "MINI DRONE");
    GunData.emplace(1015, "SUPPLY CRATE");
}

std::string Namegun::GetGunName(short gunId) {
    try {
        int adjustedId = gunId < 0 ? gunId + 25000 : gunId;
        auto it = GunData.find(adjustedId);
        if (it != GunData.end()) {
            return it->second;
        }
        return "ID: " + std::to_string(gunId);
    }
    catch (...) {
        return "Unknown";
    }
}