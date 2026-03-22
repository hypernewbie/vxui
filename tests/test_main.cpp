#include <string>

#include "../third_party/utest.h"

#define CLAY_IMPLEMENTATION
#include "../clay/clay.h"

#define VE_FONTCACHE_IMPL
#include "../vefc/ve_fontcache.h"

#define VXUI_IMPL
#include "../vxui.h"

UTEST( smoke, build_links )
{
    EXPECT_TRUE( true );
}

UTEST_MAIN();
