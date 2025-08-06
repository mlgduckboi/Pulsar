#ifndef _ITEMOBJBOMB_
#define _ITEMOBJBOMB_
#include <kamek.hpp>
#include <MarioKartWii/Item/Obj/ItemObjVariants.hpp>

//_sinit_ at 807a62d8
namespace Item {
class ObjBomb : public ObjThrowable, public ObjExplosion {
public:
    enum State {
        STATE_MOVING,
        STATE_TICKING,
        STATE_EXPLODING,
    };
    static Obj** CreateArray(u32 count); //807a41c4
    static void* SendOrExtractShootEVENT(void* packet, Obj* obj, bool extractOrSend); //807a4fa0

    ObjBomb(); //807a4850 inlined
    //ObjThrowable vtable 808d1be0 at 0x0
    void RemoveFromScn() override; //807a5b80 
    void InitSelf() override; //0x8 807a5144
    void UpdateModelPosition() override;  //0x10 807a5340
    void UpdateModelPositionNoClip() override; //0x14 807a59e0
    void SpawnModel() override; //0x18 807a5bc4
    void PrepareLight() override; //0x1c 807a5cfc
    bool SetInitialPosition(PlayerObj& playerObj) override; //0x20 807a4a18
    int OnKill() override; //0x24 807a5f6c
    void OnTetherBreak() override; //0x28 807a60f8
    void OnFinishKill() override; //0x2c 807a619c
    void vf_0x30() override; //0x30 807a500c
    void vf_0x34() override; //0x34 807a5090

    //ObjExplosion vtable 808d1c18 at 0x1a4
    ~ObjBomb() override; //thunk 807a6320 func 807a626c

    void UpdateThunk(); //807a3900
    void Update(); //807a4cdc
    // float 1a0
    // vtable 1a4
    // u16 1a8 state
    // u16? padding 1aa
    // u32 1ac nextState
    // u32 0x1b0 timer
    // u16* ptmfIdxArray 0x1b4
    // u16 padding? 0x1b6
    // ptr ptrToStructContainingPtmfArray 0x1b8
    // u32 padding 0x1bc
    // Obj* 0x1c0  pointer to self?
    Team teamId; //0x1c4
    u8 padding_0x1c8[0x1cc - 0x1c8]; 
    void* unk_ptr_0x1cc; // 0x1 model related?
    float proximityCheck;
    ModelDirector* bombCore; //0x1d4
    u8 unknown_0x1d8[0x4];
    u32 timer; //0x1dc how long a bomb lasts before exploding
    u32 otherTimer_0x1e0;
    u8 unknown_0x1e4[0x1ec - 0x1e4];
    u32 tetherBreakTimer; //0x1ec set to 0x10e on tether break
};
size_assert(ObjBomb, 0x1f0);
}//namespace Item

#endif