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

s32 GetChargeBonus(Kart::Movement* km) {
    return -(4 * (km->GetStats().mt - 31));
}

s32 GetVariableMTMaxCharge(Kart::Movement* km) {
    s32 a = Kart::maxMTCharge + GetChargeBonus(km);
    OS::Report("mt max %d\n", a);
    return a;
}
kmBranch(0x8057efe0, GetVariableMTMaxCharge);

s32 GetVariableSMTMaxCharge(Kart::Movement* km) {
    s32 a = Kart::maxSMTCharge + GetChargeBonus(km);
    OS::Report("smt max %d\n", a);
    return a;
}
kmBranch(0x8057efec, GetVariableSMTMaxCharge);

extern "C" {
asmFunc PatchMTChargeTimeKart() {
    ASM(
        nofralloc;

        lha r8, 0x3d2(r6);   // original instruction (load max mt charge)
        
        lwz r4, 0x00(r3);     // get Pointers
        lwz r4, 0x00(r4);     // get Values
        lwz r4, 0x14(r4);     // get StatsAndBsp
        lwz r4, 0x00(r4);     // get Stats
        lwz r4, 0x6C(r4);     // get miniturbo
        //add r8, r8, r8;
        //add r4, r4, r4;       // multiply miniturbo by two

        addi    r4, r4, -31;
        slwi    r4, r4, 2;

        subf r8, r4, r8;      // charge time = original - 2 * mt stat

        lis r12, 0x8057;
        ori r12, r12, 0xee80;
        mtctr r12;
        bctr;
    )
}
/*

*/
kmBranch(0x8057ee7c, PatchMTChargeTimeKart);

asmFunc PatchMTChargeTimeBike() {
    ASM(
        nofralloc;

        lha r8, 0x3d2(r6);   // original instruction (load max mt charge)

        lwz r4, 0x00(r3);     // get Pointers
        lwz r4, 0x00(r4);     // get Values
        lwz r4, 0x14(r4);     // get StatsAndBsp
        lwz r4, 0x00(r4);     // get Stats
        lwz r4, 0x6C(r4);     // get miniturbo

        //add r8, r8, r8;
        //add r4, r4, r4;       // multiply miniturbo by two

        addi    r4, r4, -31;
        slwi    r4, r4, 2;

        subf r8, r4, r8;      // charge time = original - 2 * mt stat

        lis r12, 0x8058;
        ori r12, r12, 0x88a4;
        mtctr r12;
        bctr;
    )
}
kmBranch(0x805888a0, PatchMTChargeTimeBike);
//kmCall(0x805888a0, PatchMTChargeTime);


asmFunc PatchSMTChargeTime() {
    ASM(
        nofralloc;

        lha r8, 0x3d6(r6);   // original instruction (load max smt charge)

        lwz r4, 0x00(r3);     // get Pointers
        lwz r4, 0x00(r4);     // get Values
        lwz r4, 0x14(r4);     // get StatsAndBsp
        lwz r4, 0x00(r4);     // get Stats
        lwz r4, 0x6C(r4);     // get miniturbo

        //add r8, r8, r8;
        //add r4, r4, r4;       // multiply miniturbo by two

        addi    r4, r4, -31;
        slwi    r4, r4, 2;

        subf r8, r4, r8;      // charge time = original - 2 * mt stat

        lis r12, 0x8057;
        ori r12, r12, 0xef30;
        mtctr r12;
        bctr;
    )
}
kmBranch(0x8057ef2c, PatchSMTChargeTime);
} // extern C

}
}