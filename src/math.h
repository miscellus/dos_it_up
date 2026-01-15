#ifndef MATH_H
#define MATH_H

#include <stdint.h>
#include <stddef.h>

static inline uint16_t MinU16(uint16_t a, uint16_t b) { return a < b ? a : b; }

typedef int32_t fix16;
#define FIX16_SHIFT 16
#define FIX16_ONE   (1 << FIX16_SHIFT)
#define FIX16_HALF   (1 << (FIX16_SHIFT-1))

#define FIX(a) ((a) << FIX16_SHIFT)
#define FRAC(n, d) (FIX(n)/(d))

/* Conversions */
static inline fix16 FixFromInt(int32_t n) { return (fix16)(n << FIX16_SHIFT); }
static inline int32_t FixToInt(fix16 a) { return (int32_t)(a >> FIX16_SHIFT); }

/* Basic ops */
static inline fix16 FixAdd(fix16 a, fix16 b) { return (fix16)(a + b); }
static inline fix16 FixSub(fix16 a, fix16 b) { return (fix16)(a - b); }

static inline fix16 FixMul(fix16 a, fix16 b) {
    int64_t temp = (int64_t)a * (int64_t)b;
    return (fix16)(temp >> FIX16_SHIFT);
}

static inline fix16 FixDiv(fix16 a, fix16 b) {
    /* Handle divide-by-zero defensively */
    if (b == 0) return (a >= 0) ? INT32_MAX : INT32_MIN;
    int64_t temp = ((int64_t)a << FIX16_SHIFT);
    return (fix16)(temp / b);
}

/* Faster division by constant can be implemented by caller using reciprocal */

/* Vector 2D */
typedef struct { fix16 x, y; } fixv2;

static inline fixv2 FixV2(fix16 x, fix16 y) {
    fixv2 r; r.x = x; r.y = y; return r;
}

static inline fixv2 FixV2Add(fixv2 a, fixv2 b) {
    return FixV2(FixAdd(a.x, b.x), FixAdd(a.y, b.y));
}

static inline fixv2 FixV2Sub(fixv2 a, fixv2 b) {
    return FixV2(FixSub(a.x, b.x), FixSub(a.y, b.y));
}

static inline fixv2 FixV2Scale(fixv2 v, fix16 s) {
    return FixV2(FixMul(v.x, s), FixMul(v.y, s));
}

static inline fix16 FixV2Dot(fixv2 a, fixv2 b) {
    /* dot = ax*bx + ay*by (result is fix16) */
    int64_t t1 = (int64_t)a.x * (int64_t)b.x;
    int64_t t2 = (int64_t)a.y * (int64_t)b.y;
    return (fix16)((t1 + t2) >> FIX16_SHIFT);
}

/* Integer sqrt for 32-bit unsigned integers returning fix16 (sqrt of input in fix16 units).
   We use a simple integer sqrt on a 32-bit value representing a fixed-point number scaled by FIX16_ONE.
   Input is a fix16 representing a non-negative value. */
static inline fix16 FixSqrt(fix16 a) {
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
    return (fix16)res;
}

/* Length of vector */
static inline fix16 FixV2LengthSq(fixv2 v) {
    /* compute dot then sqrt */
    int64_t t1 = (int64_t)v.x * (int64_t)v.x;
    int64_t t2 = (int64_t)v.y * (int64_t)v.y;
    int64_t sum = t1 + t2; /* sum is scaled by 2^32 (since x^2 has 32.32) */
    /* Convert sum to fix16 representation before sqrt: sum >> 16 gives 32.16, shift another 0 => we want 16.16 input to FixSqrt.
       Simpler: build a fix16 value representing length^2 by shifting sum >> FIX16_SHIFT. */
    fix16 length_sq = (fix16)(sum >> FIX16_SHIFT);
    return length_sq;
}

static inline fix16 FixV2Length(fixv2 v) {
    return FixSqrt(FixV2LengthSq(v));
}

/* Normalize vector; returns zero vector if length is zero.
   result = v / length */
static inline fixv2 FixV2Normalize(fixv2 v) {
    fix16 len = FixV2Length(v);
    if (len == 0) return FixV2(0,0);
    return FixV2(FixDiv(v.x, len), FixDiv(v.y, len));
}

/* Clamp helpers */
static inline fix16 FixClamp(fix16 v, fix16 lo, fix16 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline fix16 FixMin(fix16 a, fix16 b) {
    if (a < b) return a;
    return b;
}

static inline fix16 FixMax(fix16 a, fix16 b) {
    if (a > b) return a;
    return b;
}

/* Trig: small sine table (1024 entries) in 16.16 fixed */
#define FIX_SIN_BITS 10
#define FIX_SIN_SIZE (1 << FIX_SIN_BITS)
static const fix16 fix_sin_table[FIX_SIN_SIZE] = {
    /* python:
    ",\n".join(["{0:#06x}".format(int(sin(2*pi*i/1024) * (1<<16))) for i in range(0, 1024)])
    */
    #include "fix_sin_table.g.inc"
};

/* Sine/Cos using table lookup. Angle input is 0..FIX_SIN_SIZE-1 mapping to 0..2pi */
static inline fix16 FixSinIdx(uint32_t idx) {
    idx &= (FIX_SIN_SIZE - 1);
    return fix_sin_table[idx];
}
static inline fix16 FixCosIdx(uint32_t idx) {
    idx = (idx + (FIX_SIN_SIZE >> 2)) & (FIX_SIN_SIZE - 1);
    return fix_sin_table[idx];
}

/* Rotate a vector by table index angle: v' = [ cos -sin; sin cos ] * v */
static inline fixv2 FixV2RotateByIdx(fixv2 v, uint32_t angle_idx) {
    fix16 s = FixSinIdx(angle_idx);
    fix16 c = FixCosIdx(angle_idx);
    fix16 nx = FixSub(FixMul(v.x, c), FixMul(v.y, s));
    fix16 ny = FixAdd(FixMul(v.x, s), FixMul(v.y, c));
    return FixV2(nx, ny);
}

#endif