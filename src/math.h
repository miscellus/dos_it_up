#ifndef MATH_H
#define MATH_H

#include <stdint.h>
#include <stddef.h>

static inline uint16_t MinU16(uint16_t a, uint16_t b) { return a < b ? a : b; }

typedef int32_t fx32;
typedef struct { fx32 x, y; } fxv2;

#define FIX16_SHIFT 16
#define FIX16_ONE   (1 << FIX16_SHIFT)
#define FIX16_HALF   (1 << (FIX16_SHIFT-1))

#define fx(a) ((a) << FIX16_SHIFT)
#define fxr(n, d) (fx(n)/(d))

static inline int32_t Fx2Int(fx32 a) { return (int32_t)(a >> FIX16_SHIFT); }

static inline fx32 FxMul(fx32 a, fx32 b) {
    int64_t temp = (int64_t)a * (int64_t)b;
    return (fx32)(temp >> FIX16_SHIFT);
}

static inline fx32 FxDiv(fx32 a, fx32 b) {
    int64_t temp = ((int64_t)a << FIX16_SHIFT);
    return (fx32)(temp / b);
}

/* Faster division by constant can be implemented by caller using reciprocal */

/* Vector 2D */

static inline fxv2 FxV2(fx32 x, fx32 y) {
    fxv2 r; r.x = x; r.y = y; return r;
}

static inline fxv2 FxV2Add(fxv2 a, fxv2 b) {
    return FxV2(a.x + b.x, a.y + b.y);
}

static inline fxv2 FxV2Sub(fxv2 a, fxv2 b) {
    return FxV2(a.x - b.x, a.y - b.y);
}

static inline fxv2 FxV2Scale(fxv2 v, fx32 s) {
    return FxV2(FxMul(v.x, s), FxMul(v.y, s));
}

static inline fx32 FxV2Dot(fxv2 a, fxv2 b) {
    /* dot = ax*bx + ay*by (result is fx32) */
    int64_t t1 = (int64_t)a.x * (int64_t)b.x;
    int64_t t2 = (int64_t)a.y * (int64_t)b.y;
    return (fx32)((t1 + t2) >> FIX16_SHIFT);
}

/* Integer sqrt for 32-bit unsigned integers returning fx32 (sqrt of input in fx32 units).
   We use a simple integer sqrt on a 32-bit value representing a fixed-point number scaled by FIX16_ONE.
   Input is a fx32 representing a non-negative value. */
static inline fx32 FxSqrt(fx32 a) {
    if (a <= 0) return 0;
    /* We want sqrt(a) where a is in 16.16. Convert to 32.32 for accurate integer sqrt:
       value = a << 16 (now 32.32 fixed point stored as 48-bit but fits in 64-bit). */
    uint64_t val = ((uint64_t)(uint32_t)a) << FIX16_SHIFT; /* a * 2^16 => 32.32 */
    /* integer sqrt on val */
    uint64_t res = 0;
    uint64_t bit = (uint64_t)1 << 62; /* highest power of four <= 2^62 */
    while (bit > val) bit >>= 2;
    while (bit) {
        if (val >= res + bit) {
            val -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res = res >> 1;
        }
        bit >>= 2;
    }
    /* res is sqrt(val) in 16.16 fixed point */
    return (fx32)res;
}

/* Length of vector */
static inline fx32 FxV2LengthSq(fxv2 v) {
    /* compute dot then sqrt */
    int64_t t1 = (int64_t)v.x * (int64_t)v.x;
    int64_t t2 = (int64_t)v.y * (int64_t)v.y;
    int64_t sum = t1 + t2; /* sum is scaled by 2^32 (since x^2 has 32.32) */
    /* Convert sum to fx32 representation before sqrt: sum >> 16 gives 32.16, shift another 0 => we want 16.16 input to FxSqrt.
       Simpler: build a fx32 value representing length^2 by shifting sum >> FIX16_SHIFT. */
    fx32 length_sq = (fx32)(sum >> FIX16_SHIFT);
    return length_sq;
}

static inline fx32 FxV2Length(fxv2 v) {
    return FxSqrt(FxV2LengthSq(v));
}

/* Normalize vector; returns zero vector if length is zero.
   result = v / length */
static inline fxv2 FxV2Normalize(fxv2 v) {
    fx32 len = FxV2Length(v);
    if (len == 0) return FxV2(0,0);
    return FxV2(FxDiv(v.x, len), FxDiv(v.y, len));
}

/* Clamp helpers */
static inline fx32 FxClamp(fx32 v, fx32 lo, fx32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline fx32 FxMin(fx32 a, fx32 b) {
    if (a < b) return a;
    return b;
}

static inline fx32 FxMax(fx32 a, fx32 b) {
    if (a > b) return a;
    return b;
}

/* Trig: small sine table (1024 entries) in 16.16 fixed */
#define FIX_SIN_BITS 10
#define FIX_SIN_SIZE (1 << FIX_SIN_BITS)
static const fx32 fix_sin_table[FIX_SIN_SIZE] = {
    /* python:
    ",\n".join(["{0:#06x}".format(int(sin(2*pi*i/1024) * (1<<16))) for i in range(0, 1024)])
    */
    #include "fix_sin_table.g.inc"
};

/* Sine/Cos using table lookup. Angle input is 0..FIX_SIN_SIZE-1 mapping to 0..2pi */
static inline fx32 FxSinIdx(uint32_t idx) {
    idx &= (FIX_SIN_SIZE - 1);
    return fix_sin_table[idx];
}
static inline fx32 FxCosIdx(uint32_t idx) {
    idx = (idx + (FIX_SIN_SIZE >> 2)) & (FIX_SIN_SIZE - 1);
    return fix_sin_table[idx];
}

/* Rotate a vector by table index angle: v' = [ cos -sin; sin cos ] * v */
static inline fxv2 FxV2RotateByIdx(fxv2 v, uint32_t angle_idx) {
    fx32 s = FxSinIdx(angle_idx);
    fx32 c = FxCosIdx(angle_idx);
    fx32 nx = FxMul(v.x, c) - FxMul(v.y, s);
    fx32 ny = FxMul(v.x, s) + FxMul(v.y, c);
    return FxV2(nx, ny);
}

#endif