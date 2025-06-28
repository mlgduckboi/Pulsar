#include <kamek.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/Item/PlayerObj.hpp>
#include <Race/FrontRunFrenzy.hpp>

namespace Pulsar {
namespace Race {

ItemId DecideItem(Item::ItemSlotData* itemSlotData, u16 itemBoxType, u8 position, bool isHuman, bool hasTripleItem, Item::Player* itemHolderPlayer) {
    if (Pulsar::Race::isFrontrunFrenzy) { 
        if (position < 4) {
            return MUSHROOM;
        }
        return BLUE_SHELL;
    }
    ItemId item = itemSlotData->DecideItem(itemBoxType, position, isHuman, hasTripleItem, itemHolderPlayer);
    OS::Report("Decided %d\n", item);
    //return static_cast<ItemId>(0x20);
    return item;
}
kmCall(0x807BA160, DecideItem);

void CustomItemUseLogic(Item::PlayerObj po, bool isRemote) {
    OS::Report("item used!\n");
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