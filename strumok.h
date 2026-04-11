#ifndef STRUMOK_H
#define STRUMOK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {// тут мені порадив ші додати це, бо були помилки
#endif
#define STRUMOK_256  0
#define STRUMOK_512  1
typedef struct {
    uint64_t s[16];
    uint64_t r1;
    uint64_t r2;
    int mode;
} strumok_state_t;

void strumok_init(strumok_state_t *state,
                  const uint8_t *key,
                  const uint8_t *iv,
                  int mode);

uint64_t strumok_next_word(strumok_state_t *state);

void strumok_keystream(strumok_state_t *state, uint8_t *buf, size_t len);

void strumok_crypt(strumok_state_t *state, uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif