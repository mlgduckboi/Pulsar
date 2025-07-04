#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/Kart/KartCollision.hpp>
#include <MarioKartWii/Kart/Hitbox.hpp>

namespace Pulsar {
namespace Race {

// experiment to see if if substituting KCLs was possible (ie make all roads into jump pads). mostly functional.

void SwapFloorKCL(Kart::Collision* self, Kart::CollisionData& collisionData, Kart::Hitbox* hitbox, void* unk, 
    KCLTypeHolder& kclFlags, bool allowBoostPanels) {
        if ((kclFlags.bitfield & KCL_BITFIELD_JUMP_PAD) != 0) {
            OS::Report("jump pad with coll 0x%x\n", static_cast<u32>(kclFlags.bitfield));
        }
        
        // floor bitfield 0x20e80fff
        if (kclFlags.bitfield != 0) {
            u8 pid = self->GetPlayerIdx();
            OS::Report("player %d is on road!", pid);
            kclFlags.bitfield = KCL_BITFIELD_JUMP_PAD;
        }
        
        self->ProcessFloor(collisionData, hitbox, unk, kclFlags, allowBoostPanels);
}
//kmCall(0x8056e848, SwapFloorKCL);
//kmCall(0x8056e918, SwapFloorKCL);

bool ForceJumppadCol(KCLTypeHolder* self, KCLBitfield acceptedFlags) {
    //u8 value = 3; // any value from 0–7
    //CollisionEntry::closestEntry->kclFlag &= ~0xE0;                // clear bits 5–7
    //CollisionEntry::closestEntry->kclFlag |= (value & 0x7) << 5; 
    //CollisionEntry::closestEntry->kclFlag |= 0xE0;
    /*bool ret = self->FindClosestCollisionEntry(acceptedFlags);
    if (ret) {
        u8 value = (CollisionEntry::closestEntry->kclFlag >> 5) & 0x7;
        OS::Report("found jumppad type: %d\n", value);
    }*/
    u8 jumppadType = 3; // any value from 0–7
    CollisionEntry::closestEntry->kclFlag &= ~0xE0;                // clear bits 5–7
    CollisionEntry::closestEntry->kclFlag |= (jumppadType & 0x7) << 5; 
    //CollisionEntry::closestEntry->kclFlag |= 0xE0;
    return true;
} 
//kmCall(0x8056ed7c, ForceJumppadCol);

}//namespace Race
}//namespace Pulsar