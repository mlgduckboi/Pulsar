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
        
        // floor bitfield 0x20e80fff
        kclFlags;
        if ((kclFlags.bitfield & ~0x20e80fff) != 0) {
            //u8 pid = self->GetPlayerIdx();
            OS::Report("can be called with non road\n");
            //kclFlags.bitfield = static_cast<KCLBitfield>(kclFlags.bitfield | KCL_BITFIELD_JUMP_PAD);
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
    
    bool ret = self->FindClosestCollisionEntry(acceptedFlags);
    /*if (ret) {
        //u8 value = (CollisionEntry::closestEntry->kclFlag >> 5) & 0x7;
        //OS::Report("found jumppad type: %d\n", value);
        u8 jumppadType = 4; // any value from 0–7
        CollisionEntry::closestEntry->type.bitfield = KCL_BITFIELD_JUMP_PAD;
        CollisionEntry::closestEntry->kclFlag &= ~0xE0;                // clear bits 5–7
        CollisionEntry::closestEntry->kclFlag |= (jumppadType & 0x7) << 5; 
    }*/
    
    u8 jumppadType = 4; // any value from 0–7
    CollisionEntry::closestEntry->kclFlag &= ~0xE0;                // clear bits 5–7
    CollisionEntry::closestEntry->kclFlag |= (jumppadType & 0x7) << 5; 
    return true;
} 
//kmCall(0x8056ed7c, ForceJumppadCol);

bool CustomFindClosestCollision(KCLTypeHolder* /*self, unused*/, u32 acceptedFlags) {
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
    return false; //(closest != nullptr);
}
//kmBranch(0x807bd96c, &CustomFindClosestCollision); 

void ApplyFlagRegisterEvent(float distance, KCLTypeHolder* self, u32 bitfield, u16 kclFlag) {
    //if (bitfield != 0x100) OS::Report("registering 0x%x\n", bitfield);
    return;
    if ((bitfield & 0x20e80fff) > 0) {
        //bitfield = KCL_BITFIELD_ICY_ROAD; //static_cast<u32>(KCL_BITFIELD_JUMP_PAD);
        //kclFlag &= ~0xE0;                // clear bits 5–7
        //kclFlag |= (3 & 0x7) << 5; 
    }
    /* else {
        bitfield = static_cast<u32>(KCL_BITFIELD_STICKY_ROAD);
    }*/
    self->bitfield = static_cast<KCLBitfield>(static_cast<u32>(self->bitfield) | bitfield);
    //CollisionEntry* entry = reinterpret_cast<CollisionEntry*>(0x809C38D8); // collision entry array. this is so cursed.
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
//kmBranch(0x807bda9c, &ApplyFlagRegisterEvent);

asmFunc changeCollision() {

}

}//namespace Race
}//namespace Pulsar