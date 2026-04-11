#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "strumok.h"

static int run_test(const char *name, int mode, const uint8_t *key, const uint8_t *iv, const uint64_t *expected_Z, int verbose)
{
    strumok_state_t state;
    uint64_t Z[8];
    strumok_init(&state, key, iv, mode);
    for (int i = 0; i < 8; i++)
        Z[i] = strumok_next_word(&state);

    int ok = 1;
    for (int i = 0; i < 8; i++) {
        if (Z[i] != expected_Z[i]) {ok = 0; break; }
    }

    printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok || verbose) {
        for (int i = 0; i < 8; i++) {
            printf("Z%d: got=%016llx  exp=%016llx  %s\n", i, (unsigned long long)Z[i],(unsigned long long)expected_Z[i],(Z[i] == expected_Z[i]) ? "OK" : "MISMATCH");
        }
    }
    return ok;
}

static double benchmark(int mode, const char *label)
{
    uint8_t key[64] = {0};
    uint8_t iv[32]  = {0};
    key[0] = 0x80;

    strumok_state_t state;
    strumok_init(&state, key, iv, mode);
    const size_t CHUNK = 64;
    const size_t TOTAL = 64UL * 1024 * 1024;
    uint8_t buf[CHUNK];

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (size_t done = 0; done < TOTAL; done += CHUNK)
        strumok_keystream(&state, buf, CHUNK);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mb_per_s = (TOTAL /(1024.0 * 1024.0)) / elapsed;
    double gbit_per_s = (TOTAL * 8.0 / 1e9) / elapsed;

    printf("[BENCH] %-16s  %.1f MB/s  (%.3f Gbit/s)  elapsed=%.3fs\n",
           label, mb_per_s, gbit_per_s, elapsed);
    return mb_per_s;
}

static const uint8_t tv1_iv[32] = {0};
static const uint8_t tv1_key[32] = {
        0x80,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static const uint64_t tv1_Z[8] = {
        0xe442d15345dc66caULL, 0xf47d700ecc66408aULL,
        0xb4cb284b5477e641ULL, 0xa2afc9092e4124b0ULL,
        0x728e5fa26b11a7d9ULL, 0xe6a7b9288c68f972ULL,
        0x70eb3606de8ba44cULL, 0xaced7956bd3e3de7ULL
};

static const uint8_t tv2_iv[32] = {0};
static const uint8_t tv2_key[32] = {
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa
};
static const uint64_t tv2_Z[8] = {
        0xa7510b38c7a95d1dULL, 0xcd5ea28a15b8654fULL,
        0xc5e2e2771d0373b2ULL, 0x98ae829686d5fceeULL,
        0x45bddf65c523dbb8ULL, 0x32a93fcdd950001fULL,
        0x752a7fb588af8c51ULL, 0x9de92736664212d4ULL
};

static const uint8_t tv3_iv[32] = {
        0,0,0,0, 0,0,0,4,  0,0,0,0, 0,0,0,3,
        0,0,0,0, 0,0,0,2,0,0,0,0, 0,0,0,1
};
static const uint8_t tv3_key[32] = {
        0x80,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static const uint64_t tv3_Z[8] = {
        0xfe44a2508b5a2acdULL, 0xaf355b4ed21d2742ULL,
        0xdcd7fdd6a57a9e71ULL, 0x5d267bd2739fb5ebULL,
        0xb22eee96b2832072ULL, 0xc7de6a4cdaa9a847ULL,
        0x72d5da93812680f2ULL, 0x4a0acb7e93da2ce0ULL
};

static const uint8_t tv4_iv[32] = {
        0,0,0,0, 0,0,0,4,  0,0,0,0, 0,0,0,3,
        0,0,0,0, 0,0,0,2,0,0,0,0, 0,0,0,1
};
static const uint8_t tv4_key[32] = {
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa
};
static const uint64_t tv4_Z[8] = {
        0xe6d0efd9cea5abcdULL, 0x1e78ba1a9b0e401eULL,
        0xbcfbea2c02ba0781ULL, 0x1bd375588ae08794ULL,
        0x5493cf21e114c209ULL, 0x66cd5d7cc7d0e69aULL,
        0xa5cdb9f3380d07faULL, 0x2940d61a4d4e9ce4ULL
};

static const uint8_t tv5_iv[32] = {0};
static const uint8_t tv5_key[64] = {
        0x80,0,0,0, 0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0
};
static const uint64_t tv5_Z[8] = {
        0xf5b9ab51100f8317ULL, 0x898ef2086a4af395ULL,
        0x59571fecb5158d0bULL, 0xb7c45b6744c71fbbULL,
        0xff2efcf05d8d8db9ULL, 0x7a585871e5c419c0ULL,
        0x6b5c4691b9125e71ULL, 0xa55be7d2b358ec6eULL
};

static const uint8_t tv6_iv[32] = {0};
static const uint8_t tv6_key[64] = {
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa
};
static const uint64_t tv6_Z[8] = {
        0xd2a6103c50bd4e04ULL, 0xdc6a21af5eb13b73ULL,
        0xdf4ca6cb07797265ULL, 0xf453c253d8d01876ULL,
        0x039a64dc7a01800cULL, 0x688ce327dccb7e84ULL,
        0x41e0250b5e526403ULL, 0x9936e478aa200f22ULL
};

/* ---- D.2.1 Test 3 ---- */
/* IV = (4,3,2,1), Key 512-bit = 8000...0 */
static const uint8_t tv7_iv[32] = {
        0,0,0,0, 0,0,0,4,  0,0,0,0, 0,0,0,3,
        0,0,0,0, 0,0,0,2,0,0,0,0, 0,0,0,1
};
static const uint8_t tv7_key[64] = {
        0x80,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
        0,0,0,0,0,0,0, 0,0,0,0, 0,0,0,0
};
static const uint64_t tv7_Z[8] = {
        0xcca12eae8133aaaaULL, 0x528d85507ce8501dULL,
        0xda83c7fe3e1823f1ULL, 0x21416ebf63b71a42ULL,
        0x26d76d2bf1a625ebULL, 0xeec66ee0cd0b1efcULL,
        0x02dd68f338a345a8ULL, 0x47538790a5411adbULL
};

static const uint8_t tv8_iv[32] = {
        0,0,0,0, 0,0,0,4,0,0,0,0, 0,0,0,3,
        0,0,0,0, 0,0,0,2,0,0,0,0, 0,0,0,1
};
static const uint8_t tv8_key[64] = {
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa, 0xaa,0xaa,0xaa,0xaa
};
static const uint64_t tv8_Z[8] = {
        0x965648e775c717d5ULL, 0xa63c2a7376e92df3ULL,
        0x0b0eb0bbd47ca267ULL, 0xea593d979ae5bd39ULL,
        0xd773b5e5193cafe1ULL, 0xb0a26671d259422bULL,
        0x85b2aa326b280156ULL, 0x511ace6451435f0cULL
};

int main(void)
{
    int total = 0, passed = 0;

    printf("--- STRUMOK-256 ---\n");
    total++; passed += run_test("D.1.1 Test 1 (K=8000..0, IV=0)", STRUMOK_256, tv1_key, tv1_iv, tv1_Z, 0);
    total++; passed += run_test("D.1.1 Test 2 (K=aaa..a, IV=0)",  STRUMOK_256, tv2_key, tv2_iv, tv2_Z, 0);
    total++; passed += run_test("D.1.1 Test 3 (K=8000..0, IV=1234)", STRUMOK_256, tv3_key, tv3_iv, tv3_Z, 0);
    total++; passed += run_test("D.1.1 Test 4 (K=aaa..a, IV=1234)", STRUMOK_256, tv4_key, tv4_iv, tv4_Z, 0);

    printf("\n--- STRUMOK-512 ---\n");
    total++; passed += run_test("D.2.1 Test 1 (K=8000..0, IV=0)",    STRUMOK_512, tv5_key, tv5_iv, tv5_Z, 0);
    total++; passed += run_test("D.2.1 Test 2 (K=aaa..a, IV=0)",     STRUMOK_512, tv6_key, tv6_iv, tv6_Z, 0);
    total++; passed += run_test("D.2.1 Test 3 (K=8000..0, IV=1234)", STRUMOK_512, tv7_key, tv7_iv, tv7_Z, 0);
    total++; passed += run_test("D.2.1 Test 4 (K=aaa..a, IV=1234)",  STRUMOK_512, tv8_key, tv8_iv, tv8_Z, 0);

    if (passed < total) {
        printf("SOME TESTS FAILED - aborting benchmark\n");
        return 1;
    }

    printf("=== Benchmarks ===\n");
    benchmark(STRUMOK_256, "STRUMOK-256");
    benchmark(STRUMOK_512, "STRUMOK-512");
    printf("\n--- Extended benchmark (256 MB each) ---\n");

    for (int mode = 0; mode <= 1; mode++) {
        const char *label = (mode == STRUMOK_256) ? "STRUMOK-256" : "STRUMOK-512";
        uint8_t key[64] = {0};
        uint8_t iv[32]  = {0};
        key[0] = 0x80;

        strumok_state_t state;
        strumok_init(&state, key, iv, mode);

        const size_t TOTAL = 256UL * 1024 * 1024;
        const size_t CHUNK = 4096;
        uint8_t *buf = (uint8_t *)malloc(CHUNK);
        if (!buf) { fprintf(stderr, "malloc failed\n"); return 1; }

        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (size_t done = 0; done < TOTAL; done += CHUNK)
            strumok_keystream(&state, buf, CHUNK);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        free(buf);

        double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
        double mb_per_s   = (TOTAL / (1024.0 * 1024.0)) / elapsed;
        double gbit_per_s = (TOTAL * 8.0 / 1e9) / elapsed;
        printf("[BENCH] %-16s  %.1f MB/s  (%.3f Gbit/s)  elapsed=%.3fs  data=256MB\n",
               label, mb_per_s, gbit_per_s, elapsed);
    }

    return 0;
}
//Я  використовувала тут ші для того щоб зробити гарний вивід і виправити деякі помилки