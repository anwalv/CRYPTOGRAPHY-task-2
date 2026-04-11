#include <string.h>
#include <stdint.h>
#include "strumok.h"
#include "strumok_tables.h"

static inline uint64_t load64be(const uint8_t *p)
{
    return  ((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48) |
            ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32) |
            ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16) |
            ((uint64_t)p[6] <<  8) | ((uint64_t)p[7]);
}

static inline uint64_t mul_alpha(uint64_t x)
{
    return (x << 8) ^ strumok_alpha_mul[x >> 56];
}

static inline uint64_t mul_alphainv(uint64_t x)
{
    return (x >> 8) ^ strumok_alphainv_mul[x & 0xFF];
}

static inline uint64_t transform_T(uint64_t w)
{
    return strumok_T0[w & 0xFF] ^
           strumok_T1[(w >>  8) & 0xFF] ^
           strumok_T2[(w >> 16) & 0xFF] ^
           strumok_T3[(w >> 24) & 0xFF] ^
           strumok_T4[(w >> 32) & 0xFF] ^
           strumok_T5[(w >> 40) & 0xFF] ^
           strumok_T6[(w >> 48) & 0xFF] ^
           strumok_T7[(w >> 56) & 0xFF];
}

static inline uint64_t FSM(uint64_t x, uint64_t y, uint64_t z)
{
    return (x + y)^z;
}

static inline void next_normal(strumok_state_t *st)
{
    uint64_t new_r2 = transform_T(st->r1);
    uint64_t new_r1 = st->r2 + st->s[13];
    uint64_t new_s15 = mul_alpha(st->s[0]) ^ mul_alphainv(st->s[11]) ^ st->s[13];

    for (int j = 0; j < 15; j++)
        st->s[j] = st->s[j + 1];
    st->s[15] = new_s15;

    st->r1 = new_r1;
    st->r2 = new_r2;
}

static inline void next_init(strumok_state_t *st)
{
    uint64_t new_r2 = transform_T(st->r1);
    uint64_t new_r1 = st->r2 + st->s[13];
    uint64_t fsm_val = FSM(st->s[15], st->r1, st->r2);
    uint64_t new_s15 = fsm_val ^ mul_alpha(st->s[0]) ^ mul_alphainv(st->s[11]) ^ st->s[13];

    for (int j = 0; j < 15; j++)
        st->s[j] = st->s[j + 1];
    st->s[15] = new_s15;

    st->r1 = new_r1;
    st->r2 = new_r2;
}

static inline uint64_t strm_output(const strumok_state_t *st)
{
    return FSM(st->s[15], st->r1, st->r2) ^ st->s[0];
}

void strumok_init(strumok_state_t *state, const uint8_t *key, const uint8_t *iv, int mode)
{
    uint64_t K[8], IV[4];
    IV[3] = load64be(iv + 0);
    IV[2] = load64be(iv + 8);
    IV[1] = load64be(iv + 16);
    IV[0] = load64be(iv + 24);

    state->mode = mode;
    state->r1 = 0;
    state->r2 = 0;

    if (mode == STRUMOK_256) {
        K[3] = load64be(key + 0);
        K[2] = load64be(key + 8);
        K[1] = load64be(key + 16);
        K[0] = load64be(key + 24);
        state->s[15] = ~K[0];
        state->s[14] =  K[1];
        state->s[13] = ~K[2];
        state->s[12] =  K[3];
        state->s[11] =  K[0];
        state->s[10] = ~K[1];
        state->s[9] =  K[2];
        state->s[8] =  K[3];
        state->s[7] = ~K[0];
        state->s[6] = ~K[1];
        state->s[5] =  K[2] ^ IV[3];
        state->s[4] =  K[3];
        state->s[3] =  K[0] ^ IV[2];
        state->s[2] =  K[1] ^ IV[1];
        state->s[1] =  K[2];
        state->s[0] =  K[3] ^ IV[0];
    } else {
        K[7] = load64be(key + 0);
        K[6] = load64be(key + 8);
        K[5] = load64be(key + 16);
        K[4] = load64be(key + 24);
        K[3] = load64be(key + 32);
        K[2] = load64be(key + 40);
        K[1] = load64be(key + 48);
        K[0] = load64be(key + 56);
        state->s[15] =  K[0];
        state->s[14] = ~K[1];
        state->s[13] =  K[2];
        state->s[12] =  K[3];
        state->s[11] = ~K[7];
        state->s[10] =  K[5];
        state->s[9] = ~K[6];
        state->s[8] =  K[4] ^ IV[3];
        state->s[7] = ~K[0];
        state->s[6] =  K[1];
        state->s[5] =  K[2] ^ IV[2];
        state->s[4] =  K[3];
        state->s[3] =  K[4] ^ IV[1];
        state->s[2] =  K[5];
        state->s[1] =  K[6];
        state->s[0] =  K[7] ^ IV[0];
    }
    for (int i = 0; i < 32; i++)
        next_init(state);
    next_normal(state);
}

uint64_t strumok_next_word(strumok_state_t *state)
{
    uint64_t z = strm_output(state);
    next_normal(state);
    return z;
}

void strumok_keystream(strumok_state_t *state, uint8_t *buf, size_t len)
{
    size_t i = 0;
    while (i + 8 <= len) {
        uint64_t z = strumok_next_word(state);
        buf[i+0] = (uint8_t)(z >> 56);
        buf[i+1] = (uint8_t)(z >> 48);
        buf[i+2] = (uint8_t)(z >> 40);
        buf[i+3] = (uint8_t)(z >> 32);
        buf[i+4] = (uint8_t)(z >> 24);
        buf[i+5] = (uint8_t)(z >> 16);
        buf[i+6] = (uint8_t)(z >> 8);
        buf[i+7] = (uint8_t)(z);
        i += 8;
    }
    if (i < len) {
        uint64_t z = strumok_next_word(state);
        for (int shift = 56; i < len; i++, shift -= 8)
            buf[i] = (uint8_t)(z >> shift);
    }
}

void strumok_crypt(strumok_state_t *state, uint8_t *buf, size_t len)
{
    size_t i = 0;
    while (i + 8 <= len) {
        uint64_t z = strumok_next_word(state);
        uint64_t blk;
        memcpy(&blk, buf + i, 8);
        blk ^= z;
        buf[i+0] ^= (uint8_t)(z >> 56);
        buf[i+1] ^= (uint8_t)(z >> 48);
        buf[i+2] ^= (uint8_t)(z >> 40);
        buf[i+3] ^= (uint8_t)(z >> 32);
        buf[i+4] ^= (uint8_t)(z >> 24);
        buf[i+5] ^= (uint8_t)(z >> 16);
        buf[i+6] ^= (uint8_t)(z >> 8);
        buf[i+7] ^= (uint8_t)(z);
        i += 8;
    }

    if (i < len) {
        uint64_t z = strumok_next_word(state);
        for (int shift = 56; i < len; i++, shift -= 8)
            buf[i] ^= (uint8_t)(z >> shift);
    }
}