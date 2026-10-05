#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <Config.hpp>
#include <Settings/SettingsParam.hpp>

namespace Pulsar {

namespace Settings {

u8 Params::radioCount[Params::pageCount] ={
    3, 5, 3, 5, 2, 4, 2, 2 //menu, race, host, OTT, KO, E Settings, KO VS Settings, Item Settings
    //Add user radio count here

};
u8 Params::scrollerCount[Params::pageCount] ={ 1, 1, 1, 0, 2, 2, 1, 0 }; //menu, race, host, OTT, KO, E Settings, KO VS Settings, Item Settings

u8 Params::buttonsPerPagePerRow[Params::pageCount][Params::maxRadioCount] = //first row is PulsarSettingsType, 2nd is rowIdx of radio
{
    { 2, 2, 3, 0, 0, 0 }, //menu
    { 2, 2, 2, 2, 3, 0 }, //race
    { 2, 4, 2, 0, 0, 0 }, //host
    { 3, 3, 2, 2, 2, 0 }, //OTT
    { 2, 2, 0, 0, 0, 0 }, //KO
    { 4, 2, 2, 2, 0, 0 }, //E Settings
    { 4, 4, 0, 0, 0, 0 }, //KO VS Settings
    { 2, 2, 0, 0, 0, 0 }, //Item Settings
    //{}, //User
};

u8 Params::optionsPerPagePerScroller[Params::pageCount][Params::maxScrollerCount] =
{
    { 5, 7, 0, 0, 0}, //menu
    { 4, 0, 0, 0, 0}, //race
    { 7, 0, 0, 0, 0}, //host
    { 0, 0, 0, 0, 0}, //OTT
    { 4, 4, 0, 0, 0}, //KO
    { 10, 5, 0, 0, 0}, //E Settings
    { 5, 0, 0, 0, 0}, //KO VS Settings
    { 0, 0, 0, 0, 0}, //Item Settings
    //{}, //User
};

}//namespace Settings
}//namespace Pulsar
