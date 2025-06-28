#include <kamek.hpp>
#include <MarioKartWii/Race/Raceinfo/Raceinfo.hpp>
#include <MarioKartWii/3D/Model/ModelDirector.hpp>
#include <MarioKartWii/Kart/KartValues.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <Race/200ccParams.hpp>
#include <PulsarSystem.hpp>
#include <Settings/Settings.hpp>
#include <Race/KnockoutVS.hpp>

namespace Pulsar {
namespace Race {

RaceinfoPlayer* LoadCustomLapCount(RaceinfoPlayer* player, u8 id) {
    Pulsar::Race::numKOs;
    u8 lapCount = KMP::Manager::sInstance->stgiSection->holdersArray[0]->raw->lapCount;
    Laps lapSetting = static_cast<Laps>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_SCROLL_LAPS));
    Mode mode = static_cast<Mode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_SCROLL_MODE));
    RacedataSettings& settings = Racedata::sInstance->racesScenario.settings;
    if (settings.gamemode == MODE_VS_RACE) {
        OS::Report("Versus detected!\n");
        if (mode == MODE_KO) {
            OS::Report("KO VS Mode!\n");
            numKOs = 1 + Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_KOVS), Pulsar::SETTINGTEST_RADIO_NUMKOS);
            lapsPerKO = 1 + Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_KOVS), Pulsar::SETTINGTEST_RADIO_LAPSPERKO);
            graceLaps = Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_KOVS), Pulsar::SETTINGTEST_SCROLL_GLAPS);
            OS::Report("KO VS Mode! nKOs: %d, LPE: %d, glaps: %d\n", numKOs, lapsPerKO, graceLaps);
            totalLaps = graceLaps + ((12 / numKOs) * lapsPerKO);
            lapCount = totalLaps;
        } else if (mode == MODE_FR_FRENZY) {
            OS::Report("Frontrun Frenzy!\n");
            lapCount = 8; //TODO: make laps infinite
        } else if (int(lapSetting) > 0) {
	        OS::Report("Custom lap VS!\n");
            lapCount = static_cast<u8>(lapSetting);
        }
    }
    OS::Report("Laps: %d\n", lapCount);
    settings.lapCount = lapCount;

    return new(player) RaceinfoPlayer(id, lapCount);
}
kmCall(0x805328d4, LoadCustomLapCount);

//kmWrite32(0x80723d64, 0x7FA4EB78);
void DisplayCorrectLap(AnmTexPatHolder* texPat) { //This Anm is held by a ModelDirector in a Lakitu::Player
    register u32 maxLap;
    asm(mr maxLap, r29;);
    texPat->UpdateRateAndSetFrame((float)(maxLap - 2));
    return;
}
kmCall(0x80723d70, DisplayCorrectLap);

//kmWrite32(0x808b5cd8, 0x3F800000); //change 100cc speed ratio to 1.0    
Kart::Stats* ApplySpeedModifier(KartId kartId, CharacterId characterId) {
    union SpeedModConv {
        float speedMod;
        u32 kmpValue;
    };

    Kart::Stats* stats = Kart::ComputeStats(kartId, characterId);
    SpeedModConv speedModConv;
    speedModConv.kmpValue = (KMP::Manager::sInstance->stgiSection->holdersArray[0]->raw->speedMod << 16);
    if(speedModConv.speedMod == 0.0f) speedModConv.speedMod = 1.0f;
    float factor = System::sInstance->IsContext(PULSAR_200) ? speedFactor : 1.0f;
    factor *= speedModConv.speedMod;

    Item::greenShellSpeed = 105.0f * factor;
    Item::redShellInitialSpeed = 75.0f * factor;
    Item::redShellSpeed = 130.0f * factor;
    Item::blueShellSpeed = 260.0f * factor;
    Item::blueShellMinimumDiveDistance = 640000.0f * factor;
    Item::blueShellHomingSpeed = 130.0f * factor;

    Kart::hardSpeedCap = 120.0f * factor;
    Kart::bulletSpeed = 145.0f * factor;
    Kart::starSpeed = 105.0f * factor;
    Kart::megaTCSpeed = 95.0f * factor;

    stats->baseSpeed *= factor;
    stats->standard_acceleration_as[0] *= factor;
    stats->standard_acceleration_as[1] *= factor;
    stats->standard_acceleration_as[2] *= factor;
    stats->standard_acceleration_as[3] *= factor;

    // better low traction simulation tests for inside bikes

    /*
    float slipperyHandling = stats->handlingFactors[KCL_SLIPPERY_ROAD];
    for (int i = 0; i < 32; ++i) {
        OS::Report("Handling set to %f\n", slipperyHandling);
        stats->handlingFactors[i] = slipperyHandling;
    }

    if (stats->type == INSIDE_BIKE) {
        OS::Report("changing to outside");
        stats->type = OUTSIDE_BIKE;
        stats->targetAngle = 0;
    }
    */

    Kart::minDriftSpeedRatio = 0.55f * (factor > 1.0f ? (1.0f / factor) : 1.0f);
    Kart::unknown_70 = 70.0f * factor;
    Kart::regularBoostAccel = 3.0f * factor;

    return stats;
}
kmCall(0x8058f670, ApplySpeedModifier);

kmWrite32(0x805336B8, 0x60000000);
kmWrite32(0x80534350, 0x60000000);
kmWrite32(0x80534BBC, 0x60000000);
kmWrite32(0x80723D10, 0x281D0009);
kmWrite32(0x80723D40, 0x3BA00009);

kmWrite24(0x808AAA0C, 'PUL'); //time_number -> time_numPUL

}//namespace Race
}//namespace Pulsar