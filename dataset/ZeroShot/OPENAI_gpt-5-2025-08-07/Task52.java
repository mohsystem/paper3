import java.io.*;
import java.nio.file.*;
import java.util.Arrays;

public class Task52 {

    // ChaCha20 stream cipher implementation
    static final class ChaCha20 {
        private static final int[] SIGMA = new int[]{
                0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
        };

        private final int[] state = new int[16];
        private final byte[] buffer = new byte[64];
        private int bufferPos = 64;
        private int counter;

        public ChaCha20(byte[] key, byte[] nonce, int initialCounter) {
            if (key == null || key.length != 32) throw new IllegalArgumentException("Key must be 32 bytes");
            if (nonce == null || nonce.length != 12) throw new IllegalArgumentException("Nonce must be 12 bytes");
            state[0] = SIGMA[0];
            state[1] = SIGMA[1];
            state[2] = SIGMA[2];
            state[3] = SIGMA[3];
            for (int i = 0; i < 8; i++) {
                state[4 + i] = toIntLE(key, i * 4);
            }
            counter = initialCounter;
            state[12] = counter;
            state[13] = toIntLE(nonce, 0);
            state[14] = toIntLE(nonce, 4);
            state[15] = toIntLE(nonce, 8);
        }

        private static int rotl(int v, int c) {
            return (v << c) | (v >>> (32 - c));
        }

        private static void quarterRound(int[] x, int a, int b, int c, int d) {
            x[a] += x[b]; x[d] = rotl(x[d] ^ x[a], 16);
            x[c] += x[d]; x[b] = rotl(x[b] ^ x[c], 12);
            x[a] += x[b]; x[d] = rotl(x[d] ^ x[a], 8);
            x[c] += x[d]; x[b] = rotl(x[b] ^ x[c], 7);
        }

        private void generateBlock() {
            int[] x = Arrays.copyOf(state, 16);
            for (int i = 0; i < 10; i++) {
                quarterRound(x, 0, 4, 8, 12);
                quarterRound(x, 1, 5, 9, 13);
                quarterRound(x, 2, 6, 10, 14);
                quarterRound(x, 3, 7, 11, 15);
                quarterRound(x, 0, 5, 10, 15);
                quarterRound(x, 1, 6, 11, 12);
                quarterRound(x, 2, 7, 8, 13);
                quarterRound(x, 3, 4, 9, 14);
            }
            for (int i = 0; i < 16; i++) {
                int out = x[i] + state[i];
                toBytesLE(out, buffer, i * 4);
            }
            // increment counter
            counter = (counter + 1);
            state[12] = counter;
            bufferPos = 0;
        }

        public void xorInPlace(byte[] data, int off, int len) {
            int i = off;
            int end = off + len;
            while (i < end) {
                if (bufferPos >= 64) {
                    generateBlock();
                }
                int n = Math.min(64 - bufferPos, end - i);
                for (int j = 0; j < n; j++) {
                    data[i + j] = (byte) (data[i + j] ^ buffer[bufferPos + j]);
                }
                bufferPos += n;
                i += n;
            }
        }

        public byte[] xorBytes(byte[] data) {
            if (data == null) return null;
            byte[] out = Arrays.copyOf(data, data.length);
            xorInPlace(out, 0, out.length);
            return out;
        }

        private static int toIntLE(byte[] b, int off) {
            return (b[off] & 0xff) | ((b[off + 1] & 0xff) << 8) | ((b[off + 2] & 0xff) << 16) | ((b[off + 3] & 0xff) << 24);
        }

        private static void toBytesLE(int v, byte[] out, int off) {
            out[off] = (byte) (v);
            out[off + 1] = (byte) (v >>> 8);
            out[off + 2] = (byte) (v >>> 16);
            out[off + 3] = (byte) (v >>> 24);
        }
    }

    public static byte[] chacha20XorBytes(byte[] key, byte[] nonce, int counter, byte[] data) {
        ChaCha20 ctx = new ChaCha20(key, nonce, counter);
        return ctx.xorBytes(data);
    }

    public static boolean encryptFile(String inputPath, String outputPath, byte[] key, byte[] nonce) {
        return streamXorFile(inputPath, outputPath, key, nonce, 1);
    }

    public static boolean decryptFile(String inputPath, String outputPath, byte[] key, byte[] nonce) {
        return streamXorFile(inputPath, outputPath, key, nonce, 1);
    }

    private static boolean streamXorFile(String inputPath, String outputPath, byte[] key, byte[] nonce, int counter) {
        if (key == null || key.length != 32 || nonce == null || nonce.length != 12) return false;
        try (InputStream in = new BufferedInputStream(Files.newInputStream(Paths.get(inputPath)));
             OutputStream out = new BufferedOutputStream(Files.newOutputStream(Paths.get(outputPath)))) {
            ChaCha20 ctx = new ChaCha20(key, nonce, counter);
            byte[] buf = new byte[8192];
            int read;
            while ((read = in.read(buf)) != -1) {
                byte[] chunk = Arrays.copyOf(buf, read);
                ctx.xorInPlace(chunk, 0, read);
                out.write(chunk, 0, read);
            }
            out.flush();
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    // Helper for tests
    private static byte[] patternBytes(int n) {
        byte[] b = new byte[n];
        for (int i = 0; i < n; i++) b[i] = (byte) (i & 0xff);
        return b;
    }

    public static void main(String[] args) {
        byte[] key = new byte[32];
        for (int i = 0; i < 32; i++) key[i] = (byte) (i * 7 + 3); // fixed test key
        byte[] nonce = new byte[12];
        for (int i = 0; i < 12; i++) nonce[i] = (byte) (i * 5 + 1); // fixed test nonce

        String tmpDir = System.getProperty("java.io.tmpdir");
        String[] names = new String[]{
                "test0.txt", "test1.txt", "test2.txt", "test3.txt", "test4.txt"
        };
        int[] sizes = new int[]{0, 13, 65, 1000, 50000};
        boolean allOk = true;

        for (int t = 0; t < 5; t++) {
            String in = Paths.get(tmpDir, "java_in_" + names[t]).toString();
            String enc = Paths.get(tmpDir, "java_enc_" + names[t]).toString();
            String dec = Paths.get(tmpDir, "java_dec_" + names[t]).toString();

            byte[] content;
            if (t == 1) {
                content = "Hello, world!".getBytes();
            } else {
                content = patternBytes(sizes[t]);
            }

            try (OutputStream os = new BufferedOutputStream(new FileOutputStream(in))) {
                os.write(content);
            } catch (IOException e) {
                allOk = false;
                continue;
            }

            boolean ok1 = encryptFile(in, enc, key, nonce);
            boolean ok2 = decryptFile(enc, dec, key, nonce);

            boolean ok3 = false;
            if (ok1 && ok2) {
                try {
                    byte[] decBytes = Files.readAllBytes(Paths.get(dec));
                    ok3 = Arrays.equals(content, decBytes);
                } catch (IOException e) {
                    ok3 = false;
                }
            }
            System.out.println("Test " + t + ": " + (ok1 && ok2 && ok3 ? "OK" : "FAIL"));

            try {
                Files.deleteIfExists(Paths.get(in));
                Files.deleteIfExists(Paths.get(enc));
                Files.deleteIfExists(Paths.get(dec));
            } catch (IOException ignored) {
            }
            allOk &= (ok1 && ok2 && ok3);
        }
        System.out.println("All tests: " + (allOk ? "PASS" : "FAIL"));
    }
}