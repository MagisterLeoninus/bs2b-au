#include <AudioUnit/AudioUnit.r>

#include "bs2b-common.h"

#define RES_ID    1000
//#define COMP_TYPE 'aufx'
#define COMP_TYPE kAudioUnitType_Effect
#define COMP_SUBTYPE 'bs2b'
#define COMP_MANUF    PLUGIN_MANUFACTURER_ID
#define VERSION PLUGIN_VERSION
#define NAME "BS2B"
#define DESCRIPTION "crossfeed"
#define ENTRY_POINT "BS2BEntry"

#include "AUResources.r"
