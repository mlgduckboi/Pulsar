#include <kamek.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/Item/PlayerObj.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/UI/Section/SectionMgr.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/Input/InputManager.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>



#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Driver/DriverManager.hpp>
#include <MarioKartWii/Input/InputManager.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Effect/EffectMgr.hpp>
#include <MarioKartWii/Item/Obj/Bomb.hpp>
#include <core/egg/Math/Math.hpp>


namespace Pulsar {
namespace Race {

bool JumpLol(Input::ControllerHolder& controllerHolder) {
    const ControllerType controllerType = controllerHolder.curController->GetType();
    u16 inputs = controllerHolder.inputStates[0].buttonRaw;
    inputs = (inputs & ~controllerHolder.inputStates[1].buttonRaw);

    u16 inputsMask = 0x1000;
    
    switch(controllerType) {
        case(NUNCHUCK):
            inputsMask = 0x1000;
            break;
        case(CLASSIC):
            inputsMask = 0x1000;
            break;
        case(GCN):
            inputsMask = 0x1000;
            break;
    }
    return ((inputs & inputsMask) == inputsMask);
}

static void HandleJump() {
    const SectionPad& pad = SectionMgr::sInstance->pad;
    for (int i=0; i < Racedata::sInstance->racesScenario.localPlayerCount; ++i){
        u8 hudPlayerId = Racedata::sInstance->racesScenario.settings.hudPlayerIds[i];
        Input::RealControllerHolder* controllerHolder = &Input::Manager::sInstance->realControllerHolders[i];
        if (hudPlayerId != 0xFF){
            if (JumpLol(*controllerHolder)) {
                //Item::Manager* itemMgr = Item::Manager::sInstance;
                Raceinfo* raceinfo = Raceinfo::sInstance;
                Item::Player& player = Item::Manager::sInstance->players[hudPlayerId];
                Item::PlayerInventory& inventory = player.inventory;
                if (player.roulette.isTheRouletteSpinning && raceinfo->players[hudPlayerId]->position == 12) {
                    OS::Report("setting to crazE\n");
                    //player.roulette.OnRouletteEnd();
                    player.roulette.nextItemId = static_cast<ItemId>(0x29);
                    //player.inventory.SetItem(static_cast<ItemId>(0x29), false);
                }
                if (inventory.currentItemCount == 10) {
                    for (int i = 0; i < 12; ++i) {
                        if (i != hudPlayerId) {
                            Kart::Player* player = Kart::Manager::sInstance->players[i];
                            /*Item::Obj* newObj = itemMgr->CreateItemDirect(OBJ_BOBOMB, player->GetPosition(), player->GetSpeed(), user);
                            if (newObj != nullptr) {
                                //Item::ObjBomb* bomb = static_cast<Item::ObjBomb*>(newObj);

                                //OS::Report("new bomb?: %d\n", bomb->timer);
                                u32* value = reinterpret_cast<u32*>(reinterpret_cast<u32>(newObj) + 0x1dc);
                                u32* value2 = reinterpret_cast<u32*>(reinterpret_cast<u32>(newObj) + 0x1e0);
                                OS::Report("new bomb?: 0x%x 0x%x (pid: %d actual: %d)\n", *value, *value2, newObj->playerUsedItemId, user);
                                *value = 10;

                            }*/
                            Item::ObjBomb* newObj = static_cast<Item::ObjBomb*>(Item::Manager::sInstance->CreateItemDirect(OBJ_BOBOMB, player->GetPosition(), player->GetSpeed(), hudPlayerId));
                            u32 val = reinterpret_cast<u32>(newObj);
                            if (val > 0x100) { // returns item count if it didnt spawn #witchcraft
                                
                                OS::Report("new bomb?: addr 0x%x 0x%x 0x%x (pid: %d actual: %d)\n", val, newObj->nextState, newObj->state, newObj->playerUsedItemId, hudPlayerId);
                                newObj->OnTetherBreak();
                                newObj->nextState = Item::ObjBomb::STATE_EXPLODING;
                            }
                        }
                    }
                    inventory.ClearAll();
                }
                //Item::Manager::sInstance->players[hudPlayerId].inventory.currentItemId = TRIPLE_MUSHROOM;
                //Item::Manager::sInstance->players[hudPlayerId].inventory.currentItemCount = 3;
                /*
                Item::PlayerInventory& inventory = Item::Manager::sInstance->players[hudPlayerId].inventory;
                if (inventory.currentItemCount == 10) {
                    for (int j = 0; j < 12; ++j) {
                        if (Raceinfo::sInstance->playerIdInEachPosition[j] != hudPlayerId) {
                            Kart::Player* player = Kart::Manager::sInstance->players[Raceinfo::sInstance->playerIdInEachPosition[j]];
                            Item::Manager::sInstance->CreateItemDirect(OBJ_BOBOMB, player->GetPosition(), player->GetSpeed(), hudPlayerId);
                        }
                    }
                    inventory.ClearAll();
                }
                */
                
                //Kart::Player* playerInFirst = Kart::Manager::sInstance->players[Raceinfo::sInstance->playerIdInEachPosition[0]];
                //playerInFirst->GetCollision().ActivateOobNoKCLInfo(false, playerInFirst->IsCPU(), 0);

                //Kart::Player* player = Kart::Manager::sInstance->players[hudPlayerId];
                //Kart::Player* playerInFirst = Kart::Manager::sInstance->players[Raceinfo::sInstance->playerIdInEachPosition[0]];

                
                //Item::ObjHolder& objHolder = Item::Manager::sInstance->itemObjHolders[OBJ_MUSHROOM];


                //objHolder.Spawn(quantity, &newItem, hudPlayerId, player->GetPosition(), false);
                /*
                float tempx = player->GetPosition().x;
                float tempy = player->GetPosition().y;
                float tempz = player->GetPosition().z;
                player->GetPosition().x = playerInFirst->GetPosition().x;
                player->GetPosition().y = playerInFirst->GetPosition().y;
                player->GetPosition().z = playerInFirst->GetPosition().z;
                playerInFirst->GetPosition().x = tempx;
                playerInFirst->GetPosition().y = tempy;
                playerInFirst->GetPosition().z = tempz;
                */
            }
        }
    }
}
static RaceFrameHook HandleJumpFrame(HandleJump);
    
}
}