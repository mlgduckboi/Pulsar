#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <MarioKartWii/Race/Raceinfo/RaceInfo.hpp>
#include <Race/KnockoutVS.hpp>
#include <MarioKartWii/Race/Racedata.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/UI/Ctrl/CtrlRace/CtrlRaceTime.hpp>
#include <MarioKartWii/UI/Page/RaceHUD/RaceHUD.hpp>


namespace Pulsar {
namespace Race {

int lapsPerKO = 2; //2
int numKOs = 4; //3
int graceLaps = 0;
int totalLaps = graceLaps + ((12 / numKOs) * lapsPerKO);

void EndLapHook(RaceinfoPlayer raceInfoPlayer) {
    OS::Report("finished lap %d in place %d, with completion %f\n", raceInfoPlayer.currentLap, raceInfoPlayer.position, raceInfoPlayer.raceCompletion);
    Mode mode = static_cast<Mode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_SCROLL_MODE));
    if ((mode == MODE_KO) && (raceInfoPlayer.currentLap % lapsPerKO == 0) && (raceInfoPlayer.position > (12 - ((raceInfoPlayer.currentLap - graceLaps) / lapsPerKO) * numKOs))) {
        /*raceInfoPlayer.raceFinishTime->minutes = 60 + raceInfoPlayer.position;
	    raceInfoPlayer.raceFinishTime->seconds = 0;
	    raceInfoPlayer.raceFinishTime->milliseconds = 0;*/
        if (raceInfoPlayer.position > numKOs) {
            raceInfoPlayer.EndRace(*raceInfoPlayer.raceFinishTime, true, 0);
            raceInfoPlayer.Vanish();
            OS::Report("position %d less than %d", raceInfoPlayer.position, numKOs);
            OS::Report("player %d ko'd\n", raceInfoPlayer.id);
            return;
        }
    }
    raceInfoPlayer.EndLap(); // TODO: remove lap noise for fr frenzy
}

kmCall(0x80534fbc, EndLapHook);
kmCall(0x805350d4, EndLapHook);

void CustomPositionTracking() {
    Racedata* raceData = Racedata::sInstance;
    Raceinfo* ri = Raceinfo::sInstance;
    u8 playerCount = raceData->racesScenario.playerCount;
    Mode mode = static_cast<Mode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_SCROLL_MODE));
    RacedataSettings& settings = raceData->racesScenario.settings;
    double metrics[12];
    u8 slotToPlayer[13]; // 1‑based: slotToPlayer[1] … slotToPlayer[playerCount]

    for (u8 pi = 0; pi < playerCount; ++pi) {
        RaceinfoPlayer* pl = ri->players[pi];
        u8 pos = pl->position;
        u8 slot = pos - 1; // zero‑based index in metrics[]

        // -- Branch A: Still racing?`
        if ((pl->stateFlags & 0x02) == 0) {
            metrics[slot] = pl->raceCompletion;
        }
        // -- Branch B: Finished
        else {
            if ((mode == MODE_KO) && (settings.gamemode == MODE_VS_RACE)) {
	            metrics[slot] = double((pl->currentLap) + 1); //playerFinalCompletion[pl->id];
	        } else {
                Timer* timer = pl->raceFinishTime;
                u32 total = timer->milliseconds + (timer->seconds + (timer->minutes & 0xFF)* 60)*1000;
                if ((mode == MODE_FR_FRENZY) && (settings.gamemode == MODE_VS_RACE)) {
                    metrics[slot]  = 600000000+double(total);
                } else {
                    metrics[slot]  = 600000000-double(total);
                }
            }
            //Timer* timer = pl->raceFinishTime;
            //u32 total = timer->milliseconds + (timer->seconds + (timer->minutes & 0xFF)* 60)*1000;
            //metrics[slot]  = -double(total);

        }
        slotToPlayer[pos] = pi;
    }
    // 5. Insertion‑sort 0..playerCount‑1 of metrics[] *descending*, shuffling slotToPlayer[]
    for (int out = 1; out < (int)playerCount; ++out) {
        double keyVal = metrics[out];
        u8 keyPlr = slotToPlayer[out + 1];
        int in = out - 1;

        while (in >= 0 && metrics[in] < keyVal) {
            metrics[in + 1] = metrics[in];
            slotToPlayer[in + 2] = slotToPlayer[in + 1];
            --in;
        }
        metrics[in + 1] = keyVal;
        slotToPlayer[in + 2] = keyPlr;
    }

    for (u8 rank = 1; rank <= playerCount; ++rank) {
        u8 pidx = slotToPlayer[rank];
        RaceinfoPlayer* pl = ri->players[pidx];
        pl->position = rank;
        ri->playerIdInEachPosition[rank - 1] = pidx;
    }
    return;
}
kmBranch(0x805336d8,CustomPositionTracking);

}//namespace Race
}//namespace Pulsar