#include <kamek.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/Item/PlayerObj.hpp>
#include <Race/FrontRunFrenzy.hpp>
#include <Race/KnockoutVS.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>

namespace Pulsar {
namespace Race {

ItemId DecideItem(Item::ItemSlotData* itemSlotData, u16 itemBoxType, u8 position, bool isHuman, bool hasTripleItem, Item::Player* itemHolderPlayer) {
    if (Pulsar::Race::isFrontrunFrenzy) { 
        if (position < 4) {
            return MUSHROOM;
        }
        return BLUE_SHELL;
    }
    if (true) { // TODO: make distance based item placement a setting
        Raceinfo* ri = Raceinfo::sInstance;
        u8 pidFirst = ri->playerIdInEachPosition[0];
        u8 idxLast = 11;
        if (Pulsar::Race::isKOmode) {
            RaceinfoPlayer* rip = ri->players[itemHolderPlayer->id];
            idxLast -= (int((rip->currentLap - (graceLaps + 1)) / lapsPerKO) * numKOs);
            OS::Report("gave player on lap %d, in pos %d, a \"last place\" %d\n", rip->currentLap, position, idxLast);
        }
        u8 pidLast = ri->playerIdInEachPosition[idxLast];
        float distFromFirstToLast = ri->players[pidFirst]->raceCompletion - ri->players[pidLast]->raceCompletion;
        float positionRelative = (ri->players[pidFirst]->raceCompletion - ri->players[itemHolderPlayer->id]->raceCompletion) / distFromFirstToLast;
        position = 1 + static_cast<u8>(positionRelative * 11);
        OS::Report("relative pos of %d\n", position);
    }
    ItemId item = itemSlotData->DecideItem(itemBoxType, position, isHuman, hasTripleItem, itemHolderPlayer);
    //return static_cast<ItemId>(0x20);
    return item;
}
kmCall(0x807BA160, DecideItem);

void CustomItemUseLogic(Item::PlayerObj po, bool isRemote) {
    if (po.itemPlayer->inventory.currentItemId == 0x20) {
        po.itemPlayer->UseBullet();
        return;
    }
    po.isNotDragged = true;
    po.UseItem(isRemote);
}

kmCall(0x80795764,CustomItemUseLogic); // PlayerObj_UpdateRemote
kmCall(0x80797f94,CustomItemUseLogic); // ItemPlayer_Update

}//namespace Race
}//namespace Pulsar