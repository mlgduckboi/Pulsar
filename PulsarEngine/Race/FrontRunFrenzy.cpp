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
#include <MarioKartWii/UI/Ctrl/CtrlRace/CtrlRaceLap.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <MarioKartWii/UI/Page/RaceHUD/RaceHUD.hpp>
#include <Race/FrontRunFrenzy.hpp>
#include <Race/KnockoutVS.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Effect/EffectMgr.hpp>

namespace Pulsar {
namespace Race {

u8 prevPidInFirst = 0;
u32 times[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
u32 timestampEnteredFirst = 0;
bool isFrontrunFrenzy = false;
bool isHotPotato = false;
u8 currentHotPotato = -1;
u32 hotPotatoTimer = 0;
u8 survivorsLeft = 12;
u32 restartStarTimer = 0;

void ResetScores() {
    prevPidInFirst = 0;
    timestampEnteredFirst = -1;
    for (int i = 0; i < 12; ++i) {
        times[i] =  0;
    }
    RacedataSettings& settings = Racedata::sInstance->racesScenario.settings;
    Mode mode = static_cast<Mode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_SCROLL_MODE));
    Pulsar::Race::isFrontrunFrenzy = (settings.gamemode == MODE_VS_RACE && mode == MODE_FR_FRENZY);
    Pulsar::Race::isHotPotato = (settings.gamemode == MODE_VS_RACE && mode == MODE_HOT_POTATO);
    currentHotPotato = -1;
    hotPotatoTimer = 0;
    survivorsLeft = 12;
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
    /*if(crt->flashingFrameCounter != 0) {
        OS::Report("frames %d\n", crt->flashingFrameCounter);
    }*/
    if (!(isFrontrunFrenzy || isHotPotato)) {
        return;
    }

    crt->FillTimerGlobal(&crt->timer);
    if (isFrontrunFrenzy) {
        if (crt->timer.seconds > 10 || timestampEnteredFirst != -1) {
            int pidInFirst = Raceinfo::sInstance->playerIdInEachPosition[0];
            u32 globalTimestamp = timerToTimestamp(crt->timer);
            if (timestampEnteredFirst == -1) timestampEnteredFirst = globalTimestamp;

            if (pidInFirst != prevPidInFirst) {
                times[prevPidInFirst] += (globalTimestamp - timestampEnteredFirst) / 1000;
                if (times[prevPidInFirst] > 35) {
                    times[prevPidInFirst] = 35;
                }
                timestampEnteredFirst = globalTimestamp;
                prevPidInFirst = pidInFirst;
                /*
                for (int i = 0; i < 12; ++i) {
                    OS::Report("pid %d has score %d\n", i, times[i]);
                }
                */
                //crt->maxLap -= 1;
                crt->flashingFrameCounter = 130;
                crt->type = 3;
                crt->EnableFlashingAnimation();
            }

            u32 totalTimeInFirst = (globalTimestamp - timestampEnteredFirst) + (times[pidInFirst] * 1000);
            if (((totalTimeInFirst / 1000) >= 45) && true) {
                times[pidInFirst] = 45;
                for (int i = 0; i < 12; ++i) {
                    RaceinfoPlayer* player = Raceinfo::sInstance->players[i];
                    Timer& playerFinishTimer = *(player->raceFinishTime);
                    timestampToTimer(playerFinishTimer, times[i] * 1000);
                    player->EndRace(playerFinishTimer, true, 0);
                }
            }
            timestampToTimer(crt->timer, totalTimeInFirst);

            if (crt->timer.seconds >= 35) {
                crt->EnableFlashingAnimation();
            }
        } else {
            crt->timer.seconds += 35;
        }
        crt->timer.seconds = 45 - crt->timer.seconds;
        crt->timer.milliseconds = 0;
        crt->timer.minutes = 0;
    } else {
        u32 globalTimestamp = timerToTimestamp(crt->timer);
        if (crt->timer.seconds < 15 && currentHotPotato == 255) {
            crt->timer.seconds = 15 - crt->timer.seconds;
        } else {
            // Decide which phase we're in

            u32 intervalMs;
            /*if (survivorsLeft > 8) {          // first four eliminations
                intervalMs = 20000;             // 20 sec
            } else*/ if (survivorsLeft > 4) {   // next four
                intervalMs = 15000;             // 15 sec
            } else {                            // last four
                intervalMs = 10000;             // 10 sec
            }

            // Initialize the hotPotatoTimer at the end of grace period
            if (hotPotatoTimer == 0) {
                currentHotPotato = Raceinfo::sInstance->playerIdInEachPosition[11];
                //Kart::Manager::sInstance->players[currentHotPotato]->ActivateStarAnm();
                hotPotatoTimer = 15000 + intervalMs; // first elimination time
            }

            // Check if it's time to eliminate
            if (globalTimestamp >= hotPotatoTimer && survivorsLeft > 1) {
                RaceinfoPlayer* rip = Raceinfo::sInstance->players[currentHotPotato];
                OS::Report("current hot: %d, survivors: %d, next: %d\n", currentHotPotato, survivorsLeft - 2, Raceinfo::sInstance->playerIdInEachPosition[survivorsLeft - 2]);
                Timer& playerFinishTimer = *(rip->raceFinishTime);
                playerFinishTimer.minutes = 0;
                playerFinishTimer.milliseconds = 0;
                playerFinishTimer.seconds = survivorsLeft;
                rip->EndRace(playerFinishTimer, true, 0);
                rip->Vanish();
                survivorsLeft--;
                currentHotPotato = Raceinfo::sInstance->playerIdInEachPosition[survivorsLeft - 1];
                //Kart::Manager::sInstance->players[currentHotPotato]->ActivateStarAnm();
                hotPotatoTimer += intervalMs; // schedule next elimination
            }
            //if (globalTimestamp % 100 < 10) 
            if (restartStarTimer > 0) { restartStarTimer--; } else {
                Kart::Manager::sInstance->players[currentHotPotato]->ActivateStarAnm();
                restartStarTimer = 300;
            }

            Kart::Manager::sInstance->players[currentHotPotato]->UpdateStarAnm();
            // Calculate seconds left on current interval
            if (globalTimestamp < hotPotatoTimer) {
                crt->timer.seconds = (hotPotatoTimer - globalTimestamp) / 1000;
            }
        }
        crt->timer.milliseconds = 0;
        crt->timer.minutes = 0;
    }
    crt->SetTimer(&(crt->timer));
    return;
}
kmWritePointer(0x808d402c, &UpdateCtrlRaceTime);

void UpdateControlRaceLap(CtrlRaceLap* crl) {
    if (isFrontrunFrenzy) return;
    CtrlRaceLap::OnUpdateReal(crl);
    return;
}
kmWritePointer(0x808d3d34, &UpdateControlRaceLap);

//kmWrite32(0x80858170, 0x60000000);
kmWrite32(0x80858154, 0x2c130001); //change it to use 4 player timer in 2 player
kmWrite32(0x8085816c,0x40820008);
/*
bool AllowAllItems(Item::ItemSlotData* isd, ItemObjId objId, bool r5) {
    bool ret = isd->CanItemNotBeObtained(objId, r5);
    OS::Report("CanNotBeObtained returned %d\n", ret);
    return ret;
}
kmCall(0x8065e150, AllowAllItems);
kmCall(0x80796d30, AllowAllItems);

static bool AlwaysCapacityForItem(ItemId id) {
    bool ret = Item::Manager::IsThereCapacityForItem(id);
    OS::Report("IsThereCapacityForItem returned %d\n", ret);
    return ret;
}
kmCall(0x807ba17c, AlwaysCapacityForItem);
kmCall(0x8072fda4, AlwaysCapacityForItem);
*/

/* Taken from Brawlbox's Variety Pack */
static void ChangeBlueOBJProperties(Item::ObjProperties* dest, const Item::ObjProperties& rel){
    new (dest) Item::ObjProperties(rel);
    if (isFrontrunFrenzy) {
        dest->limit = 50;
    }
}
kmCall(0x80790b74, ChangeBlueOBJProperties);

static void ChangeBombOBJProperties(Item::ObjProperties* dest, const Item::ObjProperties& rel){
    new (dest) Item::ObjProperties(rel);
    if (isNukes){
        dest->limit = 50;
    } else {
        dest->limit = 12;
    }
}

kmCall(0x80790bb4, ChangeBombOBJProperties);

// Documented version of B_squo's code
u32 ShowTimerInMulti() {
    u32 enabled = CTRL_EXTRA_SCORE | CTRL_LAP | CTRL_ITEM_WINDOW | CTRL_RANK_NUM | CTRL_MAP | CTRL_COUNTDOWN;
    if (isFrontrunFrenzy || isHotPotato) enabled |= CTRL_TIMER;
    return enabled;
}
kmWritePointer(0x808be360, &ShowTimerInMulti); // overrides VSMultiHUD::GetEnabledCtrlRaceBases()

/*
void BlockLapEndRace(RaceinfoPlayer* rip, const Timer& finishTime, bool hasNoCameras, u32 r6) {
    if (isFrontrunFrenzy) return;
    rip->EndRace(finishTime, hasNoCameras, r6);
}

kmCall(0x80534c20, BlockLapEndRace);

/*
bool BlockForceRaceEnd(GMDataVS* gmd) {
    if (isFrontrunFrenzy) return false;
    bool ret = GMDataVS::CanRaceEndReal(gmd);
    if (ret) {
        OS::Report("end?");
    } else {
        OS::Report("dont end?");
    }
    return ret;
}

kmWritePointer(0x808b3428, &BlockForceRaceEnd);
*/

}//namespace Race
}//namespace Pulsar