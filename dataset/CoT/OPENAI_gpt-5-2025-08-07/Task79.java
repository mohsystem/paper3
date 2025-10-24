import java.nio.charset.StandardCharsets;

public class Task79 {
    // Chain-of-Through Step 1: Problem understanding
    // - Implement XTEA block cipher in CTR mode to encrypt a message using a 128-bit key and a 64-bit nonce.
    // - Function accepts inputs (message, key, nonce) and returns ciphertext hex string.
    // Chain-of-Through Step 2: Security requirements
    // - Enforce key length to be exactly 16 bytes (128-bit).
    // - Nonce must be provided (64-bit) and should be unique per message for security in CTR mode.
    // - Avoid insecure random sources by requiring explicit nonce parameter.
    // Chain-of-Through Step 3: Secure coding generation
    // - Use unsigned-like operations with masking to ensure well-defined behavior for 32-bit arithmetic.
    // - No shared mutable state; all local variables.
    // Chain-of-Through Step 4: Code review (in-code comments highlight correctness and safety considerations).
    // Chain-of-Through Step 5: Secure code output (final hardened code).

    public static String encryptMessage(String message, String key16, long nonce) {
        if (key16 == null || message == null) {
            throw new IllegalArgumentException("message and key must not be null");
        }
        byte[] keyBytes = key16.getBytes(StandardCharsets.UTF_8);
        if (keyBytes.length != 16) {
            throw new IllegalArgumentException("Key must be exactly 16 bytes (UTF-8)");
        }
        byte[] msg = message.getBytes(StandardCharsets.UTF_8);
        if (msg.length == 0) {
            return "";
        }
        int[] k = keyToWordsLE(keyBytes);
        byte[] out = new byte[msg.length];

        final int blockSize = 8;
        int blocks = (msg.length + blockSize - 1) / blockSize;
        for (int i = 0; i < blocks; i++) {
            long ctr = nonce + (long) i; // 64-bit counter with wrap
            int lo = (int) (ctr & 0xFFFFFFFFL);
            int hi = (int) ((ctr >>> 32) & 0xFFFFFFFFL);
            int[] ks = xteaEncryptBlock(lo, hi, k);
            byte[] ksBytes = intPairToBytesLE(ks[0], ks[1]); // 8 bytes keystream

            int off = i * blockSize;
            int remain = Math.min(blockSize, msg.length - off);
            for (int j = 0; j < remain; j++) {
                out[off + j] = (byte) (msg[off + j] ^ ksBytes[j]);
            }
        }
        return toHex(out);
    }

    private static int[] keyToWordsLE(byte[] key) {
        int[] w = new int[4];
        for (int i = 0; i < 4; i++) {
            int base = i * 4;
            w[i] = (key[base] & 0xFF)
                 | ((key[base + 1] & 0xFF) << 8)
                 | ((key[base + 2] & 0xFF) << 16)
                 | ((key[base + 3] & 0xFF) << 24);
        }
        return w;
    }

    private static int[] xteaEncryptBlock(int v0In, int v1In, int[] k) {
        long v0 = v0In & 0xFFFFFFFFL;
        long v1 = v1In & 0xFFFFFFFFL;
        long sum = 0L;
        final long delta = 0x9E3779B9L;
        for (int i = 0; i < 32; i++) {
            long kIdx1 = k[(int) (sum & 3)] & 0xFFFFFFFFL;
            v0 = (v0 + ((((v1 << 4) ^ (v1 >>> 5)) + v1) ^ (sum + kIdx1))) & 0xFFFFFFFFL;
            sum = (sum + delta) & 0xFFFFFFFFL;
            long kIdx2 = k[(int) ((sum >>> 11) & 3)] & 0xFFFFFFFFL;
            v1 = (v1 + ((((v0 << 4) ^ (v0 >>> 5)) + v0) ^ (sum + kIdx2))) & 0xFFFFFFFFL;
        }
        return new int[] { (int) v0, (int) v1 };
    }

    private static byte[] intPairToBytesLE(int a, int b) {
        byte[] out = new byte[8];
        // a little-endian
        out[0] = (byte) (a & 0xFF);
        out[1] = (byte) ((a >>> 8) & 0xFF);
        out[2] = (byte) ((a >>> 16) & 0xFF);
        out[3] = (byte) ((a >>> 24) & 0xFF);
        // b little-endian
        out[4] = (byte) (b & 0xFF);
        out[5] = (byte) ((b >>> 8) & 0xFF);
        out[6] = (byte) ((b >>> 16) & 0xFF);
        out[7] = (byte) ((b >>> 24) & 0xFF);
        return out;
    }

    private static String toHex(byte[] data) {
        char[] hex = new char[data.length * 2];
        final char[] digits = "0123456789abcdef".toCharArray();
        for (int i = 0; i < data.length; i++) {
            int v = data[i] & 0xFF;
            hex[i * 2] = digits[v >>> 4];
            hex[i * 2 + 1] = digits[v & 0x0F];
        }
        return new String(hex);
    }

    // 5 test cases in main
    public static void main(String[] args) {
        String[] messages = new String[] {
            "",
            "Hello, world!",
            "The quick brown fox jumps over the lazy dog",
            "Sample message with multiple blocks that exceeds eight bytes.",
            "Edge case message 1234567890!@#$%^&*()_+-=[]{}|;:',.<>/?"
        };
        String[] keys = new String[] {
            "0123456789ABCDEF",
            "A1B2C3D4E5F60708",
            "Sixteen byte key",
            "0123456789ABCDE!",
            "Uniq16ByteKey!OK"
        };
        long[] nonces = new long[] {
            0x0123456789ABCDEFL,
            0x0F1E2D3C4B5A6978L,
            0x1122334455667788L,
            0x8877665544332211L,
            0xCAFEBABEDEADBEEFL
        };
        for (int i = 0; i < 5; i++) {
            String ct = encryptMessage(messages[i], keys[i], nonces[i]);
            System.out.println("Test " + (i + 1) + " ciphertext (hex): " + ct);
        }
    }
}