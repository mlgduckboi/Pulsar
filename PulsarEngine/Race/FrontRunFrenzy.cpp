#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <MarioKartWii/Race/Raceinfo/RaceInfo.hpp>
#include <MarioKartWii/Race/Racedata.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/UI/Ctrl/CtrlRace/CtrlRaceTime.hpp>
#include <MarioKartWii/UI/Page/RaceHUD/RaceHUD.hpp>

static u8 prevPidInFirst = 0;
static u32 times[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
static u32 timestampEnteredFirst = 0;

void ResetScores() {
    prevPidInFirst = 0;
    timestampEnteredFirst = 0;
    for (int i = 0; i < 12; ++i) {
        times[i] =  0;
    }
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
    CtrlRaceTime::OnUpdateReal(crt);
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
    /*u16 ogMinutes = crt->timer.minutes;
    u8 ogSeconds = crt->timer.seconds;
    u16 ogMs = crt->timer.milliseconds;
    crt->timer.minutes = 99 - crt->timer.minutes;
    crt->timer.seconds = 59 - crt->timer.seconds;
    crt->timer.milliseconds = 999 - crt->timer.milliseconds;
    crt->SetTimer(&(crt->timer));
    crt->timer.minutes = 0;
    crt->timer.seconds = ogSeconds;
    crt->timer.milliseconds = ogMs;*/
    return;
}
kmWritePointer(0x808d402c, &UpdateCtrlRaceTime);