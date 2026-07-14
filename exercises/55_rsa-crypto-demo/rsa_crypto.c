/* 55_rsa-crypto-demo.c — RSA 公钥加密演示 (uint64 玩具版)
 *
 * 任务：实现 RSA 公钥加密的 8 个核心组件，完成一次完整的
 *       "素性判定 → 密钥生成 → 加密 → 解密 → 验证"流程。
 *
 * RSA 固定参数 (不可修改):
 *   p = 61, q = 53  (两个素数)
 *   n = p * q = 3233 (模数)
 *   φ(n) = (p-1)*(q-1) = 3120 (欧拉函数)
 *   e = 17 (公钥指数)
 *   m = 42 (明文消息，固定)
 *
 * 需实现的功能：
 *   TODO 1: gcd(a, b)             — 欧几里得算法求最大公约数
 *   TODO 2: ext_gcd(a, b, &x, &y)  — 扩展欧几里得算法，求 e 的模逆元 d
 *   TODO 3: fast_pow(base,exp,mod) — 快速模幂运算 (二分求幂)
 *   TODO 4: is_prime(num)          — Miller-Rabin 素性判定
 *   TODO 5: gen_keys(p,q,e,&n,&φ,&d) — 密钥生成，含 e 有效性验证
 *   TODO 6: encrypt(msg, e, n)     — RSA 加密：c = m^e mod n
 *   TODO 7: decrypt(cipher, d, n)  — RSA 解密：m' = c^d mod n
 *   TODO 8: main() 主流程          — 串联所有步骤并输出
 *
 * 知识点：素性判定、Miller-Rabin、非对称加密、欧拉函数、模逆元、
 *         扩展欧几里得、快速幂取模、密钥生成流程
 *
 * 验证：构建用 make；判分/自测用 clings run / clings watch；
 *       查看期望输出用 clings tests 55
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

/* ─── RSA 固定参数 ─── */
static const uint64_t p = 61;
static const uint64_t q = 53;
static const uint64_t n = 3233;   /* p * q */
static const uint64_t phi = 3120; /* (p-1)*(q-1) */
static const uint64_t e = 17;     /* 公钥指数 */
static const uint64_t m = 42;     /* 明文消息 */

/* ─── TODO 1: 最大公约数 (欧几里得算法) ─── */
static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

/* ─── TODO 2: 扩展欧几里得算法 ───
 *
 * 求整数 x, y 使得 a*x + b*y = gcd(a, b)
 * 将 x 存入 *px, y 存入 *py。
 * 返回 gcd(a, b)。
 *
 * 提示：递归实现。基准情况 b==0 时 {*px=1; *py=0; return a;}。
 *       递归调用 ext_gcd(b, a%b, &x1, &y1)，
 *       然后 *px = y1, *py = x1 - (a/b)*y1。
 */
static int64_t ext_gcd(int64_t a, int64_t b, int64_t *px, int64_t *py) {
    if (b == 0) {
        *px = 1;
        *py = 0;
        return a;
    }
    int64_t x1, y1;
    int64_t d = ext_gcd(b, a % b, &x1, &y1);
    *px = y1;
    *py = x1 - (a / b) * y1;
    return d;
}

/* ─── TODO 3: 快速模幂运算 ───
 *
 * 计算 (base^exp) % mod，使用二分求幂法 (binary exponentiation)。
 * 时间复杂度 O(log exp)。
 *
 * 提示：result = 1; base = base % mod;
 *       while (exp > 0): 若 exp 最低位为 1 → result = (result*base)%mod
 *                        exp >>= 1; base = (base*base)%mod
 */
static uint64_t fast_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) {
            result = result * base % mod;
        }
        exp >>= 1;
        base = base * base % mod;
    }
    return result;
}

/* ─── TODO 4: Miller-Rabin 素性判定 ───
 *
 * 使用 Miller-Rabin 概率素性测试判定 num 是否为素数。
 * 见证人集合 {2, 3, 5, 7, 11} 由 Jaeschke (1993) 证明对所有
 * n < 2,152,302,898,747（≈2.15×10^12）给出确定性结果；p=61、q=53 远在此范围内。
 * （注意：覆盖整个 uint64_t (n < 2^64) 需前 12 个素数基或 Sinclair 的 7 基集。）
 *
 * 算法步骤：
 *   1. 处理小情况：num < 2 → false; num ∈ {2,3} → true; num 偶数 → false
 *   2. 将 num-1 写成 d * 2^s 的形式，其中 d 为奇数
 *   3. 对每个见证人 a:
 *      a. 计算 x = a^d mod num (调用 fast_pow)
 *      b. 若 x == 1 或 x == num-1，此见证人通过，继续下一个
 *      c. 重复 s-1 次：x = x^2 mod num; 若 x == num-1 则跳出
 *      d. 若 x != num-1，返回 false (合数)
 *   4. 所有见证人通过，返回 true (素数)
 *
 * 提示：分解 n-1 的 2 的幂时使用 while ((d & 1) == 0) { d >>= 1; s++; }
 */
static int is_prime(uint64_t num) {
    if (num < 2) return 0;
    if (num == 2 || num == 3) return 1;
    if ((num & 1) == 0) return 0;

    uint64_t d = num - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        s++;
    }

    uint64_t bases[] = {2, 3, 5, 7, 11};
    for (int i = 0; i < 5; i++) {
        uint64_t a = bases[i];
        if (a % num == 0) continue;  // 基数为 num 的倍数时无意义，跳过

        uint64_t x = fast_pow(a % num, d, num);
        if (x == 1 || x == num - 1) continue;

        int cont = 0;
        for (int r = 1; r < s; r++) {
            x = fast_pow(x, 2, num);  // 等价于 x = x² mod num
            if (x == num - 1) {
                cont = 1;
                break;
            }
        }
        if (cont) continue;
        return 0;  // 合数
    }
    return 1;  // 素数
}

/* ─── TODO 5: 密钥生成 ───
 *
 * 从两个素数生成 RSA 密钥对，验证 e 的有效性，输出生成过程。
 *
 * 步骤：
 *   1. 计算 n = p*q, φ = (p-1)*(q-1)
 *   2. 按格式输出 n, φ, e
 *   3. 验证 1 < e < φ 且 gcd(e, φ) == 1（调用 gcd），否则报错返回 1
 *   4. 调用 ext_gcd 计算 d = e⁻¹ mod φ
 *   5. 输出 ext_gcd 结果、方程、d、验证 e*d mod φ == 1
 *   6. 将 n, φ, d 通过 out_n/out_phi/out_d 传出
 *   7. 成功返回 0
 *
 * 输出格式参考 clings tests 55 展示的期望输出 Step 2 部分。
 * 使用 PRIu64 / PRId64 宏格式化 uint64_t / int64_t。
 */
static int gen_keys(uint64_t prime_p, uint64_t prime_q, uint64_t pub_e, uint64_t *out_n, uint64_t *out_phi,
                    int64_t *out_d) {
    uint64_t n = prime_p * prime_q;
    uint64_t phi = (prime_p - 1) * (prime_q - 1);

    /* 验证 e 与 φ(n) 互质 */
    uint64_t g = gcd(pub_e, phi);
    if (g != 1) {
        printf("Error: gcd(e, phi) != 1\n");
        return 1;
    }

    /* 扩展欧几里得求 e 的模逆元 */
    int64_t x, y;
    int64_t g_ext = ext_gcd((int64_t)pub_e, (int64_t)phi, &x, &y);
    int64_t d = x % (int64_t)phi;
    if (d < 0) d += (int64_t)phi;  // 确保 d 为正

    printf("  n = p*q = %" PRIu64 "*%" PRIu64 " = %" PRIu64 "\n", prime_p, prime_q, n);
    printf("  φ(n) = (p-1)*(q-1) = %" PRIu64 "*%" PRIu64 " = %" PRIu64 "\n", prime_p - 1, prime_q - 1, phi);
    printf("  Select e = %" PRIu64 " (public exponent)\n", pub_e);
    printf("  Verify gcd(e, φ) = gcd(%" PRIu64 ", %" PRIu64 ") = %" PRIu64 "\n", pub_e, phi, g);
    printf("  [OK — e and φ are coprime]\n");
    printf("  ext_gcd(e=%" PRId64 ", φ=%" PRId64 ") → gcd=%" PRId64 ", x=%" PRId64 ", y=%" PRId64 "\n", (int64_t)pub_e,
           (int64_t)phi, g_ext, x, y);
    printf("  Equation: e*x + φ*y = gcd  →  %" PRId64 "*%" PRId64 " + %" PRId64 "*%" PRId64 " = %" PRId64 "\n",
           (int64_t)pub_e, x, (int64_t)phi, y, g_ext);
    printf("  Private key d = x mod φ = %" PRId64 "\n", d);

    uint64_t check = (uint64_t)((pub_e * (uint64_t)d) % phi);
    printf("  Verify: e*d mod φ = %" PRIu64 " %s\n", check, (check == 1) ? "[OK]" : "[FAIL]");

    /* 传出结果 */
    *out_n = n;
    *out_phi = phi;
    *out_d = d;
    return 0;
}

/* ─── TODO 6: RSA 加密 ─── */
static uint64_t encrypt(uint64_t msg, uint64_t pub_exp, uint64_t modulus) { return fast_pow(msg, pub_exp, modulus); }

/* ─── TODO 7: RSA 解密 ─── */
static uint64_t decrypt(uint64_t cipher, uint64_t priv_exp, uint64_t modulus) {
    return fast_pow(cipher, priv_exp, modulus);
}

/* ─── TODO 8: 主流程 ───
 *
 * 按以下步骤输出 (格式须与 clings tests 55 展示的期望输出完全一致):
 *   Step 1: 打印 p, q 并用 is_prime 验证素性，打印 n, φ, e, m
 *   Step 2: 调用 gen_keys() 完成密钥生成与验证
 *   Step 3: 加密，打印 c = m^e mod n
 *   Step 4: 解密，打印 m' = c^d mod n
 *   Step 5: 验证 m' == m，打印 RSA works! ✓
 *
 * 使用 PRIu64 / PRId64 宏格式化 uint64_t / int64_t (来自 <inttypes.h>)。
 * is_prime 返回值：1=素数，0=合数，打印 "prime ✓" 或 "composite ✗"。
 */
int main(void) {
    /* ─── 标题 ─── */
    printf("=== RSA Public-Key Crypto Demo (uint64 toy version) ===\n\n");

    /* ─── Step 1 ─── */
    printf("Step 1 — RSA Parameter Setup & Primality Verification:\n");
    printf("  Prime p = %" PRIu64 " — Miller-Rabin: %s\n", p, is_prime(p) ? "prime ✓" : "composite ✗");
    printf("  Prime q = %" PRIu64 " — Miller-Rabin: %s\n", q, is_prime(q) ? "prime ✓" : "composite ✗");
    printf("  Modulus n = p*q = %" PRIu64 "\n", n);
    printf("  Euler φ(n) = (p-1)*(q-1) = %" PRIu64 "\n", phi);
    printf("  Public exponent e = %" PRIu64 "\n", e);
    printf("  Plaintext message m = %" PRIu64 "\n", m);
    printf("\n");

    /* ─── Step 2 ─── */
    printf("Step 2 — Key Generation:\n");
    uint64_t out_n, out_phi;
    int64_t out_d;
    if (gen_keys(p, q, e, &out_n, &out_phi, &out_d) != 0) {
        printf("Key generation failed.\n");
        return 1;
    }
    printf("\n");

    /* ─── Step 3 ─── */
    printf("Step 3 — Encryption (c = m^e mod n):\n");
    uint64_t c = encrypt(m, e, out_n);
    printf("  c = %" PRIu64 "^%" PRIu64 " mod %" PRIu64 "\n", m, e, out_n);
    printf("    = fast_pow(%" PRIu64 ", %" PRIu64 ", %" PRIu64 ")\n", m, e, out_n);
    printf("    = %" PRIu64 "\n", c);
    printf("\n");

    /* ─── Step 4 ─── */
    printf("Step 4 — Decryption (m' = c^d mod n):\n");
    uint64_t m_prime = decrypt(c, out_d, out_n);
    printf("  m' = %" PRIu64 "^%" PRId64 " mod %" PRIu64 "\n", c, out_d, out_n);
    printf("     = fast_pow(%" PRIu64 ", %" PRId64 ", %" PRIu64 ")\n", c, out_d, out_n);
    printf("     = %" PRIu64 "\n", m_prime);
    printf("\n");

    /* ─── Step 5 ─── */
    printf("Step 5 — Verification:\n");
    printf("  Original message  m  = %" PRIu64 "\n", m);
    printf("  Decrypted message m' = %" PRIu64 "\n", m_prime);
    if (m_prime == m) {
        printf("  m' == m  →  RSA works! ✓\n");
    } else {
        printf("  m' != m  →  RSA failed.\n");
    }

    return 0;
}
