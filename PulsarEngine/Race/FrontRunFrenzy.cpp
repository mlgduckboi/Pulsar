#include <kamek.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/UI/Ctrl/CtrlRace/CtrlRaceBalloon.hpp>
#include <MarioKartWii/UI/Ctrl/CtrlRace/CtrlRaceResult.hpp>
#include <MarioKartWii/GlobalFunctions.hpp>
#include <MarioKartWii/Driver/DriverManager.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/Race/Raceinfo/RaceInfo.hpp>
#include <MarioKartWii/Race/Racedata.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/UI/Ctrl/CtrlRace/CtrlRaceTime.hpp>
#include <MarioKartWii/UI/Page/RaceHUD/RaceHUD.hpp>
#include <Settings/Settings.hpp>

namespace Pulsar {
namespace Race {

static u8 prevPidInFirst = 0;
static u32 times[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
static u32 timestampEnteredFirst = 0;
static bool isFrontrunFrenzy = false;

void ResetScores() {
    prevPidInFirst = 0;
    timestampEnteredFirst = 0;
    for (int i = 0; i < 12; ++i) {
        times[i] =  0;
    }
    RacedataSettings& settings = Racedata::sInstance->racesScenario.settings;
    Mode mode = static_cast<Mode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_SCROLL_MODE));
    isFrontrunFrenzy = (settings.gamemode == MODE_VS_RACE && mode == MODE_FR_FRENZY);
}
RaceLoadHook resetScores(ResetScores);

u32 timerToTimestamp(Timer& t) {
    return (((t.minutes * 60) + t.seconds) * 1000) + t.milliseconds;
}

void timestampToTimer(Timer& t, u32 timestamp) {
    t.minutes = timestamp / 60000;
    t.seconds = (timestamp % 60000) / 1000;
    t.milliseconds = (timestamp % 1000);
}

void UpdateCtrlRaceTime(CtrlRaceTime* crt) {
    if (isFrontrunFrenzy) { crt->maxLap = CtrlRaceTime::GetPlayerMaxLap(crt->playerId); } // disable lap split showing
    CtrlRaceTime::OnUpdateReal(crt);
    if (!isFrontrunFrenzy) {
        return;
    }
    int pidInFirst = Raceinfo::sInstance->playerIdInEachPosition[0];
    u32 globalTimestamp = timerToTimestamp(crt->timer);

    if (pidInFirst != prevPidInFirst) {
        times[prevPidInFirst] += (globalTimestamp - timestampEnteredFirst) / 1000;
        if (times[prevPidInFirst] > 25) {
            times[prevPidInFirst] = 25;
        }
        timestampEnteredFirst = globalTimestamp;
        prevPidInFirst = pidInFirst;
        for (int i = 0; i < 12; ++i) {
            OS::Report("pid %d has score %d\n", i, times[i]);
        }
    }

    u32 totalTimeInFirst = (globalTimestamp - timestampEnteredFirst) + (times[pidInFirst] * 1000);
    if ((totalTimeInFirst / 1000) >= 30) {
        times[pidInFirst] = 30;
        for (int i = 0; i < 12; ++i) {
            RaceinfoPlayer* player = Raceinfo::sInstance->players[i];
            Timer& playerFinishTimer = *(player->raceFinishTime);
            timestampToTimer(playerFinishTimer, times[i] * 1000);
            player->EndRace(playerFinishTimer, true, 0);
        }
    }
    timestampToTimer(crt->timer, totalTimeInFirst);
    crt->timer.seconds = 30 - crt->timer.seconds;
    crt->timer.milliseconds = 0;
    crt->SetTimer(&(crt->timer));
    return;
}
kmWritePointer(0x808d402c, &UpdateCtrlRaceTime);

}//namespace Race
}//namespace Pulsar