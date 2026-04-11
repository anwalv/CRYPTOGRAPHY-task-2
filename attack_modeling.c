#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "strumok.h"
#include "strumok_tables.h"

static inline uint64_t a_mul(uint64_t x) {
    return (x << 8) ^ strumok_alpha_mul[x >> 56];
}

static inline uint64_t ainv_mul(uint64_t x) {
    return (x >> 8) ^ strumok_alphainv_mul[x & 0xFF];
}

static inline uint64_t attack_FSM(uint64_t x, uint64_t y, uint64_t z) {
    return (x + y) ^ z;
}

static inline uint64_t transform_T_attack(uint64_t x) {
    return strumok_T0[x & 0xFF] ^
        strumok_T1[(x >> 8) & 0xFF] ^
        strumok_T2[(x >> 16) & 0xFF] ^
        strumok_T3[(x >> 24) & 0xFF] ^
        strumok_T4[(x >> 32) & 0xFF] ^
        strumok_T5[(x >> 40) & 0xFF] ^
        strumok_T6[(x >> 48) & 0xFF] ^
        strumok_T7[(x >> 56) & 0xFF];
}

static void guess_and_determine_recovery(strumok_state_t *st, const uint64_t Z[11]) {
    uint64_t R1_t[12] = {0};
    uint64_t R2_t[12] = {0};
    uint64_t S[27] = {0};

    // 7 слів базису
    S[0] = st->s[0];
    S[11] = st->s[11];
    S[12] = st->s[12];
    S[13] = st->s[13];
    S[14] = st->s[14];
    R1_t[0] = st->r1;
    R2_t[0] = st->r2;


    S[15] = ((Z[0] ^ S[0]) ^ R2_t[0]) - R1_t[0];

    for (int t = 0; t < 10; t++) {
        R2_t[t + 1] = transform_T_attack(R1_t[t]);
        R1_t[t + 1] = R2_t[t] + S[t + 13];
        S[t + 16] = a_mul(S[t]) ^ ainv_mul(S[t + 11]) ^ S[t + 13];

        uint64_t fsm_next = attack_FSM(S[t + 16], R1_t[t + 1], R2_t[t + 1]);
        S[t + 1] = Z[t + 1] ^ fsm_next;
    }

    // записуємо відновлений початковий стан назад
    for (int i = 0; i < 16; i++) {
        st->s[i] = S[i];
    }
    st->r1 = R1_t[0];
    st->r2 = R2_t[0];
}

static void print_state(const char *title, const strumok_state_t *st) {
    printf("%s\n", title);
    for (int i = 0; i < 16; i++) {
        printf("S[%2d] = %016llx\n", i, (unsigned long long)st->s[i]);
    }
    printf("R1 = %016llx\n", (unsigned long long)st->r1);
    printf("R2 = %016llx\n", (unsigned long long)st->r2);
    printf("\n");
}

void run(void) {
    printf("Струмок-512: моделювання атаки часткового вгадування\n\n");

    // 1. ініціалізація справжнього шифру
    strumok_state_t real_state, hacked_state;
    uint8_t key[64] = {0};
    uint8_t iv[32]  = {0};
    key[0] = 0x80;

    strumok_init(&real_state, key, iv, STRUMOK_512);

    // копія стану після strumok_init() — це його й треба відновити
    strumok_state_t original_state = real_state;

    // 2. перехоплення 11 слів гами
    uint64_t Z[11];
    for (int i = 0; i < 11; i++) {
        Z[i] = strumok_next_word(&real_state);
    }

    // 3. симуляція атаки - залишаємо лише 7 слів базису
    memset(&hacked_state, 0, sizeof(hacked_state));
    hacked_state.mode = STRUMOK_512;

    hacked_state.s[0] = original_state.s[0];
    hacked_state.s[11] = original_state.s[11];
    hacked_state.s[12] = original_state.s[12];
    hacked_state.s[13] = original_state.s[13];
    hacked_state.s[14] = original_state.s[14];
    hacked_state.r1 = original_state.r1;
    hacked_state.r2 = original_state.r2;

    printf("Базис із 7 слів:\n");
    printf("S[0], S[11], S[12], S[13], S[14], R1_0, R2_0\n\n");

    printf("Перехоплені 11 слів гами:\n");
    for (int i = 0; i < 11; i++) {
        printf("Z[%d] = %016llx\n", i, (unsigned long long)Z[i]);
    }
    printf("\n");

    guess_and_determine_recovery(&hacked_state, Z);

    // 4. перевірка
    int success = 1;

    for (int i = 0; i < 16; i++) {
        if (hacked_state.s[i] != original_state.s[i]) {
            success = 0;
            printf("Помилка: S[%d] recovered=%016llx, original=%016llx\n",
                   i, (unsigned long long)hacked_state.s[i], (unsigned long long)original_state.s[i]);
        }
    }

    if (hacked_state.r1 != original_state.r1) {
        success = 0;
        printf("Помилка: R1 recovered=%016llx, original=%016llx\n",
               (unsigned long long)hacked_state.r1, (unsigned long long)original_state.r1);
    }

    if (hacked_state.r2 != original_state.r2) {
        success = 0;
        printf("Помилка: R2 recovered=%016llx, original=%016llx\n",
               (unsigned long long)hacked_state.r2, (unsigned long long)original_state.r2);
    }

    if (success) {
        printf("УСПІХ! Внутрішній стан повністю відновлено з 7 64-бітних слів\n\n");
    } else {
        printf("НЕВДАЧА! Відновлення не збіглося з оригінальним станом((\n\n");
        print_state("Оригінальний стан:", &original_state);
        print_state("Відновлений стан:", &hacked_state);
    }
}

int main(void) {
    run();
    return 0;
}