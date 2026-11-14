#include "tls_mem.h"

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <mbedtls/platform.h>

// mbedtls_platform_set_calloc_free() exists when the mbedTLS memory layer is enabled and
// the allocator is not fixed at compile time (both true for the ESP-IDF build).
#if defined(BOARD_HAS_PSRAM) && defined(MBEDTLS_PLATFORM_MEMORY) && !defined(MBEDTLS_PLATFORM_CALLOC_MACRO)

// Same threshold as CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL in the precompiled libs.
static constexpr size_t TLS_PSRAM_MIN_BLOCK = 4096;

static constexpr uint32_t TLS_CAPS_INTERNAL = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
static constexpr uint32_t TLS_CAPS_PSRAM = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

static void *tlsCalloc(size_t n, size_t size) {
    size_t total = 0;
    if (__builtin_mul_overflow(n, size, &total)) return NULL;
    if (total >= TLS_PSRAM_MIN_BLOCK) {
        return heap_caps_calloc_prefer(n, size, 2, TLS_CAPS_PSRAM, TLS_CAPS_INTERNAL);
    }
    return heap_caps_calloc_prefer(n, size, 2, TLS_CAPS_INTERNAL, TLS_CAPS_PSRAM);
}

// The stock esp_mbedtls_mem_free() is heap_caps_free() too, so blocks allocated before
// the switch are released correctly by this one.
static void tlsFree(void *ptr) { heap_caps_free(ptr); }

bool tlsMemInit() {
    if (!psramFound()) return false;
    return mbedtls_platform_set_calloc_free(tlsCalloc, tlsFree) == 0;
}

#else

bool tlsMemInit() { return false; }

#endif
