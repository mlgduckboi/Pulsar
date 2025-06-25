#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <Gamemodes/OnlineTT/OnlineTT.hpp>
#include <Gamemodes/KO/KOMgr.hpp>
#include <MarioKartWii/Race/Raceinfo/Raceinfo.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/Item/PlayerObj.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Race/RaceInfo/GameModeData.hpp>

namespace Pulsar {
//For hooks which are shared by different things

namespace Race {
void UpdatePoints(RacedataScenario& scenario) {
    const GameType oldType = scenario.settings.gametype;
    const System* system = System::sInstance;
    Racedata* racedata = Racedata::sInstance;

    if(system->IsContext(PULSAR_MODE_KO) && system->koMgr->isSpectating) scenario.settings.gametype = GAMETYPE_DEFAULT;
    bool hasVSGhost = false;
    if(system->IsContext(PULSAR_MODE_OTT)) {
        if(racedata->racesScenario.players[racedata->racesScenario.playerCount - 1].playerType == PLAYER_GHOST) {
            hasVSGhost = true;
            scenario.players[racedata->racesScenario.playerCount - 1].finishPos = racedata->racesScenario.playerCount;
            racedata->racesScenario.playerCount--;
        }
    }

    /*
    for (int i = 0; i < racedata->racesScenario.playerCount; ++i) {
        racedata->pointsRoom[11][i] = 69 - i;
    }
    */

    scenario.UpdatePoints();
    racedata->menusScenario.settings.gametype = oldType;
}
kmCall(0x8085c878, UpdatePoints);

/*
bool RaceCanEnd(GMData& gmd) {
    OS::Report("thing do thing");
    if (gmd.CanRaceEnd()) {
        OS::Report("end?");
    } else {
        OS::Report("dont end?");
    }
    return false;
}
kmWritePointer(0x808b3428, RaceCanEnd);
*/

}//namespace Race
}//namespace Pulsar