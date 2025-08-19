#include <kamek.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Item/Obj/ItemObj.hpp>
#include <Race/FrontrunFrenzy.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>

namespace Pulsar {
namespace Race {

void OnPlayerItemCollision(Item::Obj* self, Kart::Player& player, bool isRemote) {
    OS::Report("Player %d hit %d\n", self->playerUsedItemId, player.GetPlayerIdx());
    self->OnPlayerCollision(player, isRemote);
    if (self->playerUsedItemId == currentHotPotato) {
        currentHotPotato = player.GetPlayerIdx();
        Kart::Manager::sInstance->players[self->playerUsedItemId]->StopStarAnm();
        restartStarTimer = 0;
        //player.ActivateStarAnm();
    }
    return;
}

kmCall(0x8079549c, OnPlayerItemCollision);
kmCall(0x80799e14, OnPlayerItemCollision);

}
}