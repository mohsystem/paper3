import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task106 {
    public static final class RSAKey {
        public final BigInteger n;
        public final BigInteger e;
        public final BigInteger d;

        public RSAKey(BigInteger n, BigInteger e, BigInteger d) {
            this.n = n;
            this.e = e;
            this.d = d;
        }
    }

    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final BigInteger PUBLIC_E = BigInteger.valueOf(65537);

    public static RSAKey generateKey(int bits) {
        if (bits < 512) throw new IllegalArgumentException("bits must be >= 512");
        BigInteger p, q, n, phi, d;
        while (true) {
            int pBits = bits / 2;
            int qBits = bits - pBits;
            p = BigInteger.probablePrime(pBits, SECURE_RANDOM);
            do {
                q = BigInteger.probablePrime(qBits, SECURE_RANDOM);
            } while (p.equals(q));

            n = p.multiply(q);
            BigInteger p1 = p.subtract(BigInteger.ONE);
            BigInteger q1 = q.subtract(BigInteger.ONE);
            phi = lcm(p1, q1);

            if (PUBLIC_E.gcd(phi).equals(BigInteger.ONE)) {
                d = PUBLIC_E.modInverse(phi);
                break;
            }
        }
        return new RSAKey(n, PUBLIC_E, d);
    }

    public static byte[] encrypt(byte[] message, BigInteger e, BigInteger n) {
        if (message == null) throw new IllegalArgumentException("message is null");
        BigInteger m = new BigInteger(1, message);
        if (m.signum() < 0 || m.compareTo(n) >= 0) {
            throw new IllegalArgumentException("Message too long for the modulus");
        }
        BigInteger c = m.modPow(e, n);
        return toFixedLength(c, (n.bitLength() + 7) / 8);
    }

    public static byte[] decrypt(byte[] ciphertext, BigInteger d, BigInteger n) {
        if (ciphertext == null) throw new IllegalArgumentException("ciphertext is null");
        BigInteger c = new BigInteger(1, ciphertext);
        if (c.signum() < 0 || c.compareTo(n) >= 0) {
            throw new IllegalArgumentException("Ciphertext representative out of range");
        }
        BigInteger m = c.modPow(d, n);
        byte[] out = m.toByteArray();
        // Strip any leading zero due to sign representation
        if (out.length > 1 && out[0] == 0) {
            out = Arrays.copyOfRange(out, 1, out.length);
        }
        return out;
    }

    private static BigInteger lcm(BigInteger a, BigInteger b) {
        return a.divide(a.gcd(b)).multiply(b);
    }

    private static byte[] toFixedLength(BigInteger x, int length) {
        byte[] raw = x.toByteArray();
        // Remove sign extension if present
        if (raw.length > 1 && raw[0] == 0) {
            raw = Arrays.copyOfRange(raw, 1, raw.length);
        }
        if (raw.length > length) {
            throw new IllegalArgumentException("Value does not fit into target length");
        }
        byte[] out = new byte[length];
        System.arraycopy(raw, 0, out, length - raw.length, raw.length);
        return out;
    }

    public static void main(String[] args) {
        // Generate a 2048-bit key (recommended). For faster tests reduce to 1024.
        RSAKey key = generateKey(2048);

        String[] tests = new String[] {
            "Hello RSA!",
            "",
            "The quick brown fox jumps over the lazy dog",
            "1234567890",
            "🔒 RSA Test 🚀"
        };

        for (int i = 0; i < tests.length; i++) {
            byte[] msg = tests[i].getBytes(StandardCharsets.UTF_8);
            byte[] ct = encrypt(msg, key.e, key.n);
            byte[] pt = decrypt(ct, key.d, key.n);
            String recovered = new String(pt, StandardCharsets.UTF_8);
            System.out.println("Test " + (i + 1) + ": " + (recovered.equals(tests[i]) ? "OK" : "FAIL"));
        }
    }
}