#include <kamek.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/Item/PlayerObj.hpp>
#include <Race/FrontRunFrenzy.hpp>
#include <Race/KnockoutVS.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Driver/DriverManager.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/Input/InputManager.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Effect/EffectMgr.hpp>
#include <MarioKartWii/Item/Obj/Bomb.hpp>
#include <core/egg/Math/Math.hpp>
//#include <MarioKartWii/Effect/PlayerEffects.hpp>

namespace Pulsar {
namespace Race {

static u8 worldUser = 0;
static u32 worldTimer = 0;

static u32 frogTimer = 0;

static u8 backwardsBUser = 0;
static u32 backwardsBTimer = 0;
static bool invertedThisTick[12];

enum CustomItemId {
    ENDERPEARL         = 0x21,
    BABY_OIL           = 0x22,
    DEATHNOTE_ULTIMATE = 0x23,
    BACKWARDS_B        = 0x24,
    NUKE_BUTTON        = 0x25,
    NUKE_MEGA          = 0x26,
    NUKE_REVERSE       = 0x27,
    FORG               = 0x28,
    CRAZE              = 0x29,
    DEATHNOTE          = 0x2A,
    WORLD              = 0x2B,
    GOLDEN_SHELL       = 0x2C,
    GOLDEN_BANANA      = 0x2D,
    ULTRACUT           = 0x2E,
    BOOM_SHROOM        = 0x2F
};

static u32 oilTimer = 0;

static void UpdateTimers() {
    if (worldTimer > 0) {
        --worldTimer;
    }
    if (frogTimer > 0) {
        --frogTimer;
    }
    if (oilTimer > 0) {
        --oilTimer;
    }
    if (backwardsBTimer > 0) {
        --backwardsBTimer;
        if (backwardsBTimer == 60) {
            for (int i = 0; i < 12; i++) {
                if (i != backwardsBUser) {
                    //Kart::Manager::sInstance->players[i]->GetDamage().ApplyPOWDamage(0xC);
                    Effects::Player* player = Effects::Mgr::sInstance->players[i];
                    player->CreateSpinEffect(Kart::Damage::SPIN_TYPE_SHORT, true);
                }
            }
        }
    }
    for (int i = 0; i < 12; ++i) {
        invertedThisTick[i] = false;

        //Item::Manager::sInstance->players[i].roulette.SetUnkItem(THUNDER_CLOUD);
        Item::Player* player = &(Item::Manager::sInstance->players[i]);
        Item::PlayerInventory& inventory = player->inventory;
        if (inventory.currentItemId == 0x29 && inventory.currentItemCount > 1
            && (player->pointers->kartStatus->bitfield2 & 0x8000000) == 0) {
            inventory.currentItemCount = 1;
            inventory.currentItemId = static_cast<ItemId>(0x23);
        }

        if (inventory.currentItemId == 0x2c || inventory.currentItemId == 0x2d) {
            if (inventory.goldenTimer > 0) {
                inventory.goldenTimer--;
                if (inventory.goldenTimer == 0) inventory.ClearAll();
            }
        }
        if (oilTimer == 1) {
            Kart::ScaleAnmController* scalePlayer = player->GetMovement().scaleController;
            OS::Report("state: %d\n", scalePlayer->status);
            scalePlayer->RequestEndScale(false);
            //if (scalePlayer->status == 1) {
            //    scalePlayer->status = static_cast<Kart::ScaleAnmController::Status>(-1);//RequestScaleChange(true);
            //}
        }
    }
}
RaceFrameHook CustomItemTimers(UpdateTimers);

void EnableFrogMode(float distance, KCLTypeHolder* self, u32 bitfield, u16 kclFlag) {
    if ((frogTimer > 0) && ((bitfield & 0x20e80fff) > 0)) {
        bitfield = static_cast<u32>(KCL_BITFIELD_JUMP_PAD);
        kclFlag &= ~0xE0;                // clear bits 5–7
        kclFlag |= (4 & 0x7) << 5; 
    }
    /* else {
        bitfield = static_cast<u32>(KCL_BITFIELD_STICKY_ROAD);
    }*/
    self->bitfield = static_cast<KCLBitfield>(static_cast<u32>(self->bitfield) | bitfield);
    CollisionEntry* entry = &CollisionEntry::entriesArray;
    if (CollisionEntry::entryCount > 63) {
        CollisionEntry::entryCount = 63;
    }
    entry += CollisionEntry::entryCount;
    entry->type.bitfield = static_cast<KCLBitfield>(bitfield);
    entry->kclFlag = kclFlag;
    entry->distance = distance;
    CollisionEntry::entryCount++;
    return;
}
kmBranch(0x807bda9c, &EnableFrogMode);

bool EnableBabyOil(KCLTypeHolder* /*self, unused*/, u32 acceptedFlags) {
    CollisionEntry* closest = nullptr;
    CollisionEntry::closestEntry = nullptr;

    CollisionEntry* entry = &CollisionEntry::entriesArray; 
    float bestD = CollisionEntry::maxDist;
    u32 count = CollisionEntry::entryCount;
    //return false;
    while (count > 0)
    {
        if ((static_cast<u32>(entry->type.bitfield) & acceptedFlags) != 0
            && entry->distance > bestD) {
            closest = entry;
            bestD = entry->distance;
            CollisionEntry::closestEntry = entry;
        }
        count--;
        entry += 1;
    }

    /*if (closest != nullptr && CollisionEntry::closestEntry->type.bitfield != KCL_BITFIELD_JUMP_PAD) {
        OS::Report("found col of type: 0x%x\n", static_cast<u32>(CollisionEntry::closestEntry->type.bitfield));
    }*/
    /*if (acceptedFlags == KCL_BITFIELD_JUMP_PAD) {
        u8 jumppadType = 4; // any value from 0–7
        CollisionEntry::closestEntry->kclFlag &= ~0xE0;                // clear bits 5–7
        CollisionEntry::closestEntry->kclFlag |= (jumppadType & 0x7) << 5;
        return true;
    }*/
    return (closest != nullptr) && (oilTimer == 0 || frogTimer > 0);
}
kmBranch(0x807bd96c, &EnableBabyOil); 

Input::ControllerHolder& GetControllerHolder(Kart::Link *link) {
    u8 pid = link->pointers->values->playerIdx;
    Input::ControllerHolder* ch = Raceinfo::sInstance->players[pid]->realControllerHolder;
    if (pid != worldUser && worldTimer > 0) {
        ch->inputStates[0].quantisedStickX = 0; //14 - ch->inputStates[0].quantisedStickX;
        ch->inputStates[0].quantisedStickY = 0; //14 - ch->inputStates[0].quantisedStickY;
        ch->inputStates[0].stick.x = 0; //-1;
        ch->inputStates[0].stick.z = 0; //-1;
    } else if (pid != backwardsBUser && backwardsBTimer > 0) {
        if (!invertedThisTick[pid]) {
            Input::State& state = ch->inputStates[0];
            state.quantisedStickX = 14 - state.quantisedStickX;
            state.quantisedStickY = 14 - state.quantisedStickY;
            state.stick.x *= -1;
            state.stick.z *= -1;
            invertedThisTick[pid] = true;
        }
    }
    return *ch;
}
kmBranch(0x805903f4, GetControllerHolder);

void ConditionalStickInvert(Input::RealControllerHolder* rch, bool unk) {
    OS::Report("called\n");
    rch->Update(unk);
    Input::State& state = rch->inputStates[0];
    state.quantisedStickX = 14 - state.quantisedStickX;
    state.quantisedStickY = 14 - state.quantisedStickY;
    state.stick.x *= -1;
    state.stick.z *= -1;
}
//kmCall(0x80521784, ConditionalStickInvert);
//kmCall(0x80521a1c, ConditionalStickInvert);
//kmWritePointer(0x808b2dc0, ConditionalStickInvert); // this will prob break nstc

void CustomDecideItem(Item::Player* ip, u16 playerItemBoxType, u16 cpuItemBoxType, u32 lotteryType) {
    if (isNukes) {
        Item::PlayerInventory& inventory = ip->inventory;
        if (inventory.currentItemCount == 0) {
            inventory.SetItem(BOBOMB, false);
            inventory.currentItemCount = 0;
        }
        inventory.currentItemCount += (1 + DriverMgr::GetRaceinfoRandom()->NextLimited(3));
        if (inventory.currentItemCount > 10) {
            inventory.currentItemCount = 10;
        }
        OS::Report("now %d bombs\n", inventory.currentItemCount);
    } else {
        ip->DecideItem(playerItemBoxType, cpuItemBoxType, lotteryType);
    }
}
kmCall(0x80828da4, CustomDecideItem);
kmCall(0x80828d70, CustomDecideItem);


void notDragged(Item::Player& ip) {
    ip.playerObj.isNotDragged = true;
}

// taken from brawlbox vp
static void ChangeItemBehaviour(){
    NoDrag nodrag = static_cast<NoDrag>(
        Pulsar::Settings::Mgr::Get().GetUserSettingValue(
            static_cast<Pulsar::Settings::UserType>(
                Pulsar::Settings::SETTINGSTYPE_TEST
            ), 
            Pulsar::SETTINGTEST_RADIO_NODRAG
        )
    );
    Item::Behavior *table = Item::Behavior::behaviourTable;
    if (isFrontrunFrenzy){
        table[BLUE_SHELL].useType = Item::ITEMUSE_CIRCLE;
        table[BLUE_SHELL].numberOfItems = 0x3;

        table[BULLET_BILL].objId = OBJ_BLUE_SHELL;
        table[BULLET_BILL].useType = Item::ITEMUSE_CIRCLE;
        table[BULLET_BILL].numberOfItems = 0x3;

        table[STAR].objId = OBJ_BLUE_SHELL;
        table[STAR].useType = Item::ITEMUSE_CIRCLE;
        table[STAR].numberOfItems = 0x1;

        table[POW].objId = OBJ_BLUE_SHELL;
        table[POW].useType = Item::ITEMUSE_CIRCLE;
        table[POW].numberOfItems = 0x3;

        table[MEGA_MUSHROOM].objId = OBJ_BLUE_SHELL;
        table[MEGA_MUSHROOM].useType = Item::ITEMUSE_CIRCLE;
        table[MEGA_MUSHROOM].numberOfItems = 0x1;

        table[BLOOPER].objId = OBJ_BLUE_SHELL;
        table[BLOOPER].useType = Item::ITEMUSE_CIRCLE;
        table[BLOOPER].numberOfItems = 0x1;

        table[LIGHTNING].objId = OBJ_BLUE_SHELL;
        table[LIGHTNING].useType = Item::ITEMUSE_CIRCLE;
        table[LIGHTNING].numberOfItems = 0x1;

        //table[TRIPLE_GREEN_SHELL].objId = OBJ_BLUE_SHELL;
        //table[TRIPLE_RED_SHELL].objId = OBJ_BLUE_SHELL;
    } else if (nodrag == NODRAG_ENABLED) {
        table[RED_SHELL].useFunction = &notDragged;
        table[GREEN_SHELL].useFunction = &notDragged;
        table[BANANA].useFunction = &notDragged;
        table[TRIPLE_BANANA].useType = Item::ITEMUSE_CIRCLE;
    }
}
kmBranch(0x807bd1cc, ChangeItemBehaviour);

ItemId DecideItem(Item::ItemSlotData* itemSlotData, u16 itemBoxType, u8 position, bool isHuman, bool hasTripleItem, Item::Player* itemHolderPlayer) {
    Raceinfo* ri = Raceinfo::sInstance;
    u8 idxLast = 11;
    if (Pulsar::Race::isKOmode) {
        RaceinfoPlayer* rip = ri->players[itemHolderPlayer->id];
        idxLast -= (int((rip->currentLap - (graceLaps + 1)) / lapsPerKO) * numKOs);
    }
    RelItem relitem = static_cast<RelItem>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_RADIO_RELITEM));
    if (relitem == RELITEM_ENABLED) { // TODO: make distance based item placement a setting
        u8 pidFirst = ri->playerIdInEachPosition[0];
        u8 pidLast = ri->playerIdInEachPosition[idxLast];
        float distFromFirstToLast = ri->players[pidFirst]->raceCompletion - ri->players[pidLast]->raceCompletion;
        float positionRelative = (ri->players[pidFirst]->raceCompletion - ri->players[itemHolderPlayer->id]->raceCompletion) / distFromFirstToLast;
        position = 1 + static_cast<u8>(positionRelative * 11);
    }/* else {
        position = (position / (idxLast + 1)) * 12;
    }*/
    ItemId item = itemSlotData->DecideItem(itemBoxType, position, (isHuman || isFrontrunFrenzy), hasTripleItem, itemHolderPlayer);
    
    CustomItem customItem = static_cast<CustomItem>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_TEST), Pulsar::SETTINGTEST_RADIO_CUSTOMITEM));
    if (!isFrontrunFrenzy && customItem == CUSTOMITEM_ENABLED) {
        Random* random = DriverMgr::GetRaceinfoRandom();
        //s32 chance = 20 + ((position - 1) * 2); //20-45%
        //if (!isHuman) chance = 7; //1-2.25% for bots
        /*s32 chance = 101;
        if (random->NextLimited(100) < chance) {
            u32 itemId = 0x21 + random->NextLimited(14);
            if (itemId >= 0x23) {
                itemId++;
            }
            if (isHuman || itemId != 0x29) {    // ban crazE from bots
                item = static_cast<ItemId>(itemId);
            }
        }*/
        if (isHuman && random->NextLimited(100) < 50) {
            // custom item distr logic
            u32 itemId;
            bool isValid = false;
            while (!isValid) {
                isValid = true;
                itemId = 0x21 + (random->NextLimited(56) / 4);
                if (itemId >= DEATHNOTE_ULTIMATE) { // skip ult death note
                    itemId++;
                }
                if ((backwardsBTimer + worldTimer + frogTimer + oilTimer > 0) 
                && (itemId == WORLD || 
                    itemId == FORG || 
                    itemId == BACKWARDS_B || 
                    itemId == BABY_OIL)) isValid = false;
            }
            if (itemId == NUKE_REVERSE && position > 8) itemId = NUKE_BUTTON;
            else if (itemId == NUKE_BUTTON && position < 5) itemId = NUKE_REVERSE;
            item = static_cast<ItemId>(itemId); 
        } else {
            item = static_cast<ItemId>(random->NextLimited(0x13));
        }
    }   
    //return static_cast<ItemId>(DEATHNOTE);
    //return static_cast<ItemId>(0x2f);
    else if (Pulsar::Race::isFrontrunFrenzy) { 
        if (item == GREEN_SHELL || item == BANANA) {
            item = MUSHROOM;
        } else if (item == TRIPLE_BANANA) {
            item = TRIPLE_MUSHROOM;
        } else if (item == FAKE_ITEM_BOX) {
            item = BLUE_SHELL;
        } else if (item == BOBOMB) {
            item = BLUE_SHELL;
        } else if (item == TRIPLE_GREEN_SHELL || item == TRIPLE_RED_SHELL) {
            item = BLUE_SHELL;
        }
    }
    return item;
}
kmCall(0x807BA160, DecideItem);

void SwapRaceinfoPlayerProgress(RaceinfoPlayer* a, RaceinfoPlayer* b) {
    // u16
    u16 temp_u16;
    temp_u16 = a->checkpoint;
    a->checkpoint = b->checkpoint;
    b->checkpoint = temp_u16;

    // float
    float temp_f;
    temp_f = a->raceCompletion;
    a->raceCompletion = b->raceCompletion;
    b->raceCompletion = temp_f;

    temp_f = a->raceCompletionMax;
    a->raceCompletionMax = b->raceCompletionMax;
    b->raceCompletionMax = temp_f;

    temp_f = a->firstKcpLapCompletion;
    a->firstKcpLapCompletion = b->firstKcpLapCompletion;
    b->firstKcpLapCompletion = temp_f;

    temp_f = a->nextCheckpointLapCompletion;
    a->nextCheckpointLapCompletion = b->nextCheckpointLapCompletion;
    b->nextCheckpointLapCompletion = temp_f;

    temp_f = a->nextCheckpointLapCompletionMax;
    a->nextCheckpointLapCompletionMax = b->nextCheckpointLapCompletionMax;
    b->nextCheckpointLapCompletionMax = temp_f;

    // u8
    u8 temp_u8;
    temp_u8 = a->respawn;
    a->respawn = b->respawn;
    b->respawn = temp_u8;

    // u16
    temp_u16 = a->currentLap;
    a->currentLap = b->currentLap;
    b->currentLap = temp_u16;

    // u8
    temp_u8 = a->maxLap;
    a->maxLap = b->maxLap;
    b->maxLap = temp_u8;

    temp_u8 = a->currentKCP;
    a->currentKCP = b->currentKCP;
    b->currentKCP = temp_u8;
}

void UseSwap(Item::PlayerObj& po) {
    Kart::Player* playerInFirst = Kart::Manager::sInstance->players[Raceinfo::sInstance->playerIdInEachPosition[0]];

    Quat userRot = po.GetPhysics().mainRot;
    Quat targetRot = playerInFirst->GetPhysics().mainRot;
    Vec3 userPos = po.GetPosition();
    Vec3 targetPos = playerInFirst->GetPosition();
    userPos.y += 15;
    targetPos.y += 15;

    po.GetPhysics().Reset();
    playerInFirst->GetPhysics().Reset();
    po.pointers->kartMovement->Reset(true, true);
    playerInFirst->pointers.kartMovement->Reset(true, true);
    /*{
        float tempx = po.GetPosition().x;
        float tempy = po.GetPosition().y;
        float tempz = po.GetPosition().z;
        po.GetPosition().x = playerInFirst->GetPosition().x;
        po.GetPosition().y = playerInFirst->GetPosition().y;
        po.GetPosition().z = playerInFirst->GetPosition().z;
        playerInFirst->GetPosition().x = tempx;
        playerInFirst->GetPosition().y = tempy;
        playerInFirst->GetPosition().z = tempz;
    }*/

    po.SetKartRotation(targetRot);
    playerInFirst->SetKartRotation(userRot);
    po.SetKartPosition(targetPos);
    playerInFirst->SetKartPosition(userPos);

    RaceinfoPlayer* ripUser = Raceinfo::sInstance->players[po.GetPlayerIdx()];
    RaceinfoPlayer* ripFirst = Raceinfo::sInstance->players[playerInFirst->GetPlayerIdx()];
    SwapRaceinfoPlayerProgress(ripUser, ripFirst);
}

void UseDeathnote(Item::PlayerObj& po) {
    
    s8 positionAhead = Raceinfo::sInstance->players[po.GetPlayerIdx()]->position - 2;
    OS::Report("pos ahead: %d\n", positionAhead);
    if (positionAhead < 0) positionAhead = 11;
    Kart::Player* playerInFront = Kart::Manager::sInstance->players[
        Raceinfo::sInstance->playerIdInEachPosition[positionAhead]
    ];
    playerInFront->GetCollision().ActivateOobNoKCLInfo(false, playerInFront->IsCPU(), 0);
}

void UseUltimateDeathnote(Item::PlayerObj& po) {
    for (int i = 0; i < 12; ++i) {
        if (i == po.pointers->values->playerIdx) continue;
        Kart::Player* playerToKill = Kart::Manager::sInstance->players[i];
        playerToKill->GetCollision().ActivateOobNoKCLInfo(false, playerToKill->IsCPU(), 0);
    }
}

void UseWorld(Item::PlayerObj& po) {
    worldTimer = 300;
    worldUser = po.GetPlayerIdx();
}

void UseBackwardsB(Item::PlayerObj& po) {
    u8 pid = po.GetPlayerIdx();
    backwardsBUser = pid;
    if (backwardsBTimer == 0) {
        for (int i = 0; i < 12; i++) {
            if (i != pid) {
                //Kart::Manager::sInstance->players[i]->GetDamage().ApplyPOWDamage(0xC);
                Effects::Player* player = Effects::Mgr::sInstance->players[i];
                player->CreateSpinEffect(Kart::Damage::SPIN_TYPE_LONG, true);
            }
        }
    }
    backwardsBTimer = 600;
}

void UseForg(Item::PlayerObj& po) {
    frogTimer = 600;
}

void NukePositions(u8 startPos, u8 numberOfPlayersToNuke, u8 user) {
    Kart::Manager* kartMgr = Kart::Manager::sInstance;
    Item::Manager* itemMgr = Item::Manager::sInstance;
    Raceinfo* raceinfo = Raceinfo::sInstance;
    for (int i = 0; i < numberOfPlayersToNuke; ++i) {
        u8 pos = i + startPos;
        if (pos < 0 || pos > 11) continue;
        u8 currentPid = raceinfo->playerIdInEachPosition[pos];
        if (currentPid != user) {
            Kart::Player* player = kartMgr->players[currentPid];
            /*Item::Obj* newObj = itemMgr->CreateItemDirect(OBJ_BOBOMB, player->GetPosition(), player->GetSpeed(), user);
            if (newObj != nullptr) {
                //Item::ObjBomb* bomb = static_cast<Item::ObjBomb*>(newObj);

                //OS::Report("new bomb?: %d\n", bomb->timer);
                u32* value = reinterpret_cast<u32*>(reinterpret_cast<u32>(newObj) + 0x1dc);
                u32* value2 = reinterpret_cast<u32*>(reinterpret_cast<u32>(newObj) + 0x1e0);
                OS::Report("new bomb?: 0x%x 0x%x (pid: %d actual: %d)\n", *value, *value2, newObj->playerUsedItemId, user);
                *value = 10;

            }*/
            Item::ObjBomb* newObj = static_cast<Item::ObjBomb*>(Item::Manager::sInstance->CreateItemDirect(OBJ_BOBOMB, player->GetPosition(), player->GetSpeed(), user));
            u32 val = reinterpret_cast<u32>(newObj);
            if (val > 0x100) { 
                OS::Report("new bomb?: 0x%x 0x%x (pid: %d actual: %d)\n", newObj->nextState, newObj->state, newObj->playerUsedItemId, user);
                newObj->OnTetherBreak();
                newObj->nextState = Item::ObjBomb::STATE_EXPLODING;
            }
        }
    }
}

void UseMegaNuke(Item::PlayerObj& po) {
    NukePositions(0, 12, po.GetPlayerIdx());
}

void UseReverseNuke(Item::PlayerObj& po) {
    u8 userPos = Raceinfo::sInstance->players[po.GetPlayerIdx()]->position - 1;
    NukePositions(userPos + 1, 3, po.GetPlayerIdx());
}

void UseNuke(Item::PlayerObj& po) {
    u8 userPos = Raceinfo::sInstance->players[po.GetPlayerIdx()]->position - 1;
    NukePositions(userPos - 3, 3, po.GetPlayerIdx());
}

void UseCrazE(Item::PlayerObj& po) {
    Item::PlayerInventory& pi = po.itemPlayer->inventory;
    if ((po.pointers->kartStatus->bitfield2 & 0x8000000) == 0) {
        if (pi.currentItemCount == 1) po.GetMovement().ActivateBullet(0xff);
    } else {
        UseMegaNuke(po);
    }
    pi.currentItemCount = 3;
}

void UseGoldenShell(Item::PlayerObj& po) {
    Item::PlayerInventory& pi = po.itemPlayer->inventory;
    
    u16 timer = pi.goldenTimer;
    if (timer == 0) {
        timer = 300;
    }

    pi.SetItem(GREEN_SHELL, false);
    po.isNotDragged = true;
    po.UseItem(false);
    pi.currentItemId = static_cast<ItemId>(0x2c);
    pi.currentItemCount = 3;

    pi.goldenTimer = timer;
}

void UseGoldenBanana(Item::PlayerObj& po) {
    Item::PlayerInventory& pi = po.itemPlayer->inventory;

    u16 timer = pi.goldenTimer;
    if (timer == 0) {
        timer = 300;
    }
    
    pi.SetItem(BANANA, false);
    po.isNotDragged = true;
    po.UseItem(false);
    pi.currentItemId = static_cast<ItemId>(0x2d);
    pi.currentItemCount = 3;

    pi.goldenTimer = timer;
}

void UseUltracut(Item::PlayerObj& po) {
    RaceinfoPlayer* player = Raceinfo::sInstance->players[po.pointers->values->playerIdx];
    player->EndLap();
}

void UseBoomShroom(Item::PlayerObj& po) {
    //UseBoomShroom(po);

    po.GetMovement().ActivateMushroom();
    
    u8 user = po.GetPlayerIdx();
    Kart::Player* player = Kart::Manager::sInstance->players[user];
    Vec3 vel = player->GetSpeed();
    //float mag = Math::Sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
    Vec3 sum;
    Kart::Physics* physics = po.pointers->kartBody->kartPhysicsHolder->physics;
    MTX::PSVECAdd(&physics->engineSpeed, &physics->speed2, &sum);
    MTX::PSVECAdd(&physics->speed3, &sum, &sum);
    float speed = MTX::PSVECMag(&sum);
    vel.x = -vel.x;
    vel.y = -vel.y;
    vel.z = -vel.z;
    OS::Report("speed: %f\n", speed);
    Item::ObjBomb* newObj = static_cast<Item::ObjBomb*>(Item::Manager::sInstance->CreateItemDirect(OBJ_BOBOMB, player->GetPosition(), vel, user));
    u32 val = reinterpret_cast<u32>(newObj);
    if (val > 0x100) { 

        OS::Report("new bomb?: 0x%x 0x%x (pid: %d actual: %d)\n", newObj->nextState, newObj->state, newObj->playerUsedItemId, user);
        newObj->OnTetherBreak();
        newObj->nextState = Item::ObjBomb::STATE_EXPLODING;
        if (speed < 60.0) {
            newObj->nextState = Item::ObjBomb::STATE_TICKING;
        }
        /*newObj->tetherBreakTimer = 300;
        newObj->delayBeforeGroundEffect = 0;
        newObj->stateTimer = 0;
        newObj->timer = 0;
        newObj->otherTimer_0x1e0 = 0;*/
        //newObj->OnPlayerCollision(*player, false);

    }
}

void UseBabyOil(Item::PlayerObj& po) {
    OS::Report("called\n");
    oilTimer = 450;

    for (int i = 0; i < 12; ++i) {
        if (i == po.GetPlayerIdx()) continue;
        Kart::Player* player = Kart::Manager::sInstance->players[i];
        player->GetMovement().scaleController->RequestScaleChange(false);
    }
    //Vec3 scale;
    //scale.x = 1.25;
    //scale.y = 1;
    //scale.z = 1.25;
    //po.GetMovement().scaleController->curScale = scale;
    //po.GetMovement().scaleController->RequestScaleChange(false);
}

void CustomItemUseLogic(Item::PlayerObj& po, bool isRemote) {
    if (isNukes) {
        u32 itemCount = --po.itemPlayer->inventory.currentItemCount;
        po.itemPlayer->inventory.currentItemCount = 1;
        po.UseItem(isRemote);
        if (itemCount > 0) {
            po.itemPlayer->inventory.SetItem(BOBOMB, false);
            po.itemPlayer->inventory.currentItemCount = itemCount;
        }
        return;
    }
    if (po.itemPlayer->inventory.currentItemId >= 0x20) {
        if (po.itemPlayer->inventory.currentItemId == 0x20) {
            OS::Report("hanachan/fireball/0x20\n");
        } /*else if (po.itemPlayer->inventory.currentItemId == 0x21) {
            OS::Report("honeBall/wiggler/0x21\n");
            //po.GetPhysics().fullRot;
        } else if (po.itemPlayer->inventory.currentItemId == 0x22) {
            OS::Report("wanwan/chomp/0x22\n");
        } */
        
        else if (po.itemPlayer->inventory.currentItemId == 0x23) { //deathnote ultimate
            UseUltimateDeathnote(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x24) { //backwards_b
            UseBackwardsB(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x25) { //nuke_button
            UseNuke(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x26) { //nuke_mega
            UseMegaNuke(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x27) { //nuke_reverse
            UseReverseNuke(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x28) { //forg
            UseForg(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x29) { //crazE
            UseCrazE(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x2a) { //deathnote
            UseDeathnote(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x2b) { //world
            UseWorld(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x2c) { //gold shell
            UseGoldenShell(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x2d) { //gold banana
            UseGoldenBanana(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x2e) { //ultracut
            UseUltracut(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x2f) { //boom shroom
            UseBoomShroom(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x22) { //baby oil
            UseBabyOil(po);
        } else if (po.itemPlayer->inventory.currentItemId == 0x21) { //enderpearl
            UseSwap(po);
        }
        
        OS::Report("%d items on use\n", po.itemPlayer->inventory.currentItemCount);
        po.itemPlayer->inventory.RemoveItems(1);
        return;
    }
    po.isNotDragged = false;
    //ItemId cur = po.itemPlayer->inventory.currentItemId;
    //if (cur == RED_SHELL || cur == GREEN_SHELL || cur == BANANA) po.isNotDragged = true;
    po.UseItem(isRemote);
}

kmCall(0x80795764,CustomItemUseLogic); // PlayerObj_UpdateRemote
kmCall(0x80797f94,CustomItemUseLogic); // ItemPlayer_Update

kmWrite32(0x80791a38, 0x60000000); // noop isNotDraggedFlag

void CustomItemCount(Item::PlayerInventory* pi) {
    //OS::Report("item count assigned: %d\n", pi->currentItemCount);
    if (pi->currentItemId >= 0x20) {
        if (pi->currentItemId == 0x2f) {
            pi->currentItemCount = 3;
        } else {
            pi->currentItemCount = 1;
        }
    }
    return;
}
kmBranch(0x807bc978, CustomItemCount);

void itemRegisterCalled() {
    OS::Report("ObjHolder_registerNew() called\n");
}
//kmBranch(0x80796344, itemRegisterCalled);

void registerLostItemCalled(Item::PlayerInventory* pi) {
    OS::Report("other lose item called! %d\n", pi->currentItemId);
    if (pi->currentItemId >= 0x20) {
        pi->ClearAll();
        return;
    }
    pi->RegisterLostItems();
}
//kmCall(0x807bc72c, registerLostItemCalled);

void loseItemOnDmg(Item::PlayerInventory* pi) {
    OS::Report("dmg lose item called! %d\n", pi->currentItemId);
    if (isNukes) {
        if (pi->currentItemCount > 7) {
            pi->currentItemCount = 3;
        } else if (pi->currentItemCount > 4) {
            pi->currentItemCount = 2;
        } else if (pi->currentItemCount > 1) {
            pi->currentItemCount = 1;
        }
    }
    //if (pi->currentItemId >= 0x20) return;
    if (pi->currentItemId >= 0x20) {
        pi->ClearAll();
        return;
    }
    pi->LoseItemFromDmg();
}
kmCall(0x80798aac,loseItemOnDmg);

}//namespace Race
}//namespace Pulsar