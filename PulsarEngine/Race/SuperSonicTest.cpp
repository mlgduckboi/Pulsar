#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <MarioKartWii/Race/Raceinfo/RaceInfo.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartValues.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartPointers.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>

namespace Pulsar {
namespace Race {

// mostly a cpp version of clf's fkw rampup with minor modifications
/*
void ApplyNewFactorToPlayer(Kart::Stats* stats, float factor) {
    //stats->baseSpeed *= factor;
    stats->standard_acceleration_as[0] *= factor;
    stats->standard_acceleration_as[1] *= factor;
    stats->standard_acceleration_as[2] *= factor;
    stats->standard_acceleration_as[3] *= factor;
}

void ApplyNewFactorGlobal(float factor) {
    Item::greenShellSpeed *= factor;
    Item::redShellInitialSpeed *= factor;
    Item::redShellSpeed *= factor;
    Item::blueShellSpeed *= factor;
    Item::blueShellMinimumDiveDistance *= factor;
    Item::blueShellHomingSpeed *= factor;

    Kart::hardSpeedCap *= factor;
    Kart::bulletSpeed *= factor;
    Kart::starSpeed *= factor;
    Kart::megaTCSpeed *= factor;
    Kart::unknown_70 *= factor;
    Kart::regularBoostAccel *= factor;
}


// Port of CLF's from FKW
void DynamicHardSpeedCap(Kart::Movement* movement) {
    movement->hardSpeedLimit = Kart::hardSpeedCap * movement->speedMultiplier;
    OS::Report("hard speed upd: %f\n", movement->hardSpeedLimit);
    return;
}
//kmBranch(0x8057b9ac, DynamicHardSpeedCap);

void RampUpSpeed(RaceinfoPlayer raceInfoPlayer) {
    Kart::Stats* stats = Kart::Manager::sInstance->players[raceInfoPlayer.id]->values->statsAndBsp.stats;
    Kart::Movement* movement = Kart::Manager::sInstance->players[raceInfoPlayer.id]->pointers.kartMovement;
    if (raceInfoPlayer.currentLap > 0) {
        float factor = movement->speedMultiplier;
        movement->speedMultiplier += 0.5;
        factor = movement->speedMultiplier / factor;
        movement->baseSpeed *= factor;
        ApplyNewFactorToPlayer(stats, factor);
        
        DynamicHardSpeedCap(movement);
        if (raceInfoPlayer.position == 1) ApplyNewFactorGlobal(factor);
        OS::Report("%f\n", factor);
    }
    OS::Report("hardspeedcap: %f (player %d: %f soft: %f)\n", Kart::hardSpeedCap, raceInfoPlayer.id, movement->hardSpeedLimit, movement->softSpeedLimit);
    OS::Report("min drift speed %f\n", Kart::minDriftSpeedRatio);
}
*/

}//namespace Race
}//namespace Pulsar