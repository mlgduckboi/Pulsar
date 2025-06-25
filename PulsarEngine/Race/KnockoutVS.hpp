#ifndef _PUL_KOVS_
#define _PUL_KOVS_

namespace Pulsar {
namespace Race {
static int lapsPerKO = 2; //2
static int numKOs = 4; //3
static int graceLaps = 0;
static int totalLaps = graceLaps + ((12 / numKOs) * lapsPerKO);

}//namespace Race
}//namespace Pulsar
#endif