#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartValues.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartPointers.hpp>
#include <MarioKartWii/Kart/KartLink.hpp>
#include <MarioKartWii/Input/ControllerHolder.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>

// MOVED TO CUSTOMITEM
Input::ControllerHolder& GetControllerHolder(Kart::Link *link) {
    Input::ControllerHolder* ch = Raceinfo::sInstance->players[
                                    link->pointers->values->playerIdx
                                ]->realControllerHolder;
    ch->inputStates[0].quantisedStickX = 0; //14 - ch->inputStates[0].quantisedStickX;
    ch->inputStates[0].quantisedStickY = 0; //14 - ch->inputStates[0].quantisedStickY;
    ch->inputStates[0].stick.x *= 0; //-1;
    ch->inputStates[0].stick.z *= 0; //-1;
                                
    return *ch;
}
//kmBranch(0x805903f4, GetControllerHolder); 

