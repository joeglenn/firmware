#ifndef __TLS_MEM_H__
#define __TLS_MEM_H__
/*
    mbedTLS memory on PSRAM boards.

    The precompiled Arduino / ESP-IDF libraries are built with
    CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC, so every mbedTLS allocation comes from internal
    DRAM, even with megabytes of PSRAM free. A TLS session needs two ~17 kB record
    buffers before the handshake starts (they are static: the dynamic-buffer option is
    unavailable because DTLS is enabled), plus the handshake itself. With Wi-Fi up, a
    T-Embed CC1101 has ~69 kB of internal DRAM free with a ~31 kB largest block, and
    every TLS connect (WiFiClientSecure, HTTPClient, the JS wifi.httpFetch) fails
    straight away with "connection refused".

    tlsMemInit() installs an mbedTLS allocator, through the supported
    mbedtls_platform_set_calloc_free() hook, that follows the rule the general heap
    already uses on these boards (CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=4096):

      - blocks of 4096 bytes or more: PSRAM first, internal DRAM as the fallback
      - smaller blocks: internal DRAM first, exactly as before; PSRAM only if that fails

    So the small, hot allocations (bignums, keys, contexts: Wi-Fi WPA, BLE pairing, SSH)
    stay where they were, and only the large TLS buffers move.

    Nothing changes on boards built without BOARD_HAS_PSRAM, or when PSRAM is not found
    at boot: the function returns false and the stock allocator stays in place.
*/

// Call once, early in setup(), after psramInit() and before anything uses TLS.
// Returns true when the PSRAM-aware allocator was installed.
bool tlsMemInit();

#endif // __TLS_MEM_H__
