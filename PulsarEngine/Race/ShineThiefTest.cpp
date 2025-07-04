#include <kamek.hpp>

namespace Pulsar {
namespace Race {

// shine thief coin battle would be epic but the class that handles coin battle has like no documentation ;-;

extern u32 CoinMgr_MakeCoins(u8* gameState, int* coinConfigPtr); // Makes coin spawners

u32 TestCoinCall(u8* gameState, int* coinConfigPtr) {
    OS::Report("spawn coin called\n");
    return CoinMgr_MakeCoins(gameState, coinConfigPtr);
}
kmCall(0x808277a0, TestCoinCall);

}//namespace Race
}//namespace Pulsar