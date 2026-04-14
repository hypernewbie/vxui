#include "utest.h"
#include "../vxui_impl_util.h"
#include <glm/glm.hpp>

/* ---- hash ------------------------------------------------------------ */

UTEST(hash, nonempty) {
    ASSERT_NE((uint32_t)0, vxui_hash("hello"));
}

UTEST(hash, deterministic) {
    ASSERT_EQ(vxui_hash("options"), vxui_hash("options"));
}

UTEST(hash, different_strings_differ) {
    ASSERT_NE(vxui_hash("title"), vxui_hash("options"));
}

/* ---- arena ----------------------------------------------------------- */

UTEST(arena, create) {
    uint8_t buf[1024];
    vxui_alloc_state a = vxui_alloc_create(sizeof(buf), buf);
    ASSERT_EQ(a.base, buf);
    ASSERT_EQ(a.size, (uint64_t)sizeof(buf));
    ASSERT_EQ(a.offset, (uint64_t)0);
}

UTEST(arena, alloc_basic) {
    uint8_t buf[1024];
    vxui_alloc_state a = vxui_alloc_create(sizeof(buf), buf);
    void* p = vxui_alloc(&a, 64, 8);
    ASSERT_NE(p, (void*)NULL);
    ASSERT_EQ((uintptr_t)p % 8, (uintptr_t)0);  /* aligned */
}

UTEST(arena, alloc_oom) {
    uint8_t buf[64];
    vxui_alloc_state a = vxui_alloc_create(sizeof(buf), buf);
    void* p = vxui_alloc(&a, 128, 1);
    ASSERT_EQ(p, (void*)NULL);  /* out of memory */
}

UTEST(arena, reset) {
    uint8_t buf[1024];
    vxui_alloc_state a = vxui_alloc_create(sizeof(buf), buf);
    vxui_alloc(&a, 256, 1);
    ASSERT_NE(a.offset, (uint64_t)0);
    vxui_alloc_reset(&a);
    ASSERT_EQ(a.offset, (uint64_t)0);
}

/* ---- spring ---------------------------------------------------------- */

// at rest: value == target, velocity == 0 -> stays exactly put
UTEST(spring, at_rest) {
    glm::vec2 s = { 5.0f, 0.0f };
    vxui_spring_update(s, 5.0f, 12.0f, 1.0f / 60.0f);
    ASSERT_NEAR(s.x, 5.0f, 1e-6f);
    ASSERT_NEAR(s.y, 0.0f, 1e-6f);
}

// converges: after enough steps, value reaches target
UTEST(spring, converges) {
    glm::vec2 s = { 0.0f, 0.0f };
    for (int i = 0; i < 300; i++)
        vxui_spring_update(s, 1.0f, 12.0f, 1.0f / 60.0f);
    ASSERT_NEAR(s.x, 1.0f, 1e-3f);
    ASSERT_NEAR(s.y, 0.0f, 1e-3f);
}

// no overshoot: critically damped means value never exceeds target (approaching from below)
UTEST(spring, no_overshoot) {
    glm::vec2 s = { 0.0f, 0.0f };
    for (int i = 0; i < 300; i++) {
        vxui_spring_update(s, 1.0f, 12.0f, 1.0f / 60.0f);
        ASSERT_TRUE(s.x <= 1.001f);
    }
}

// retarget: changing target mid-flight still converges
UTEST(spring, retarget) {
    glm::vec2 s = { 0.0f, 0.0f };
    for (int i = 0; i < 10; i++)
        vxui_spring_update(s, 1.0f, 12.0f, 1.0f / 60.0f);
    for (int i = 0; i < 300; i++)
        vxui_spring_update(s, 0.0f, 12.0f, 1.0f / 60.0f);
    ASSERT_NEAR(s.x, 0.0f, 1e-3f);
    ASSERT_NEAR(s.y, 0.0f, 1e-3f);
}

UTEST_MAIN();
