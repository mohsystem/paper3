import java.io.ByteArrayOutputStream;
import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.interfaces.RSAPrivateCrtKey;
import java.util.Base64;

public class Task78 {

    // Generates an RSA private key and returns it in OpenSSH private key format as a String.
    public static String generateOpenSSHRSAPrivateKey(int keySize, String comment) throws Exception {
        int safeKeySize = Math.max(2048, keySize);
        SecureRandom rng = SecureRandom.getInstanceStrong();

        KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
        kpg.initialize(safeKeySize, rng);
        KeyPair kp = kpg.generateKeyPair();

        RSAPrivateCrtKey priv = (RSAPrivateCrtKey) kp.getPrivate();

        // Build outer structure
        ByteArrayOutputStream outer = new ByteArrayOutputStream();
        outer.write("openssh-key-v1\0".getBytes(StandardCharsets.UTF_8)); // magic

        putString(outer, "none"); // ciphername
        putString(outer, "none"); // kdfname
        putString(outer, new byte[0]); // kdfoptions

        putUint32(outer, 1); // number of keys

        // public key
        byte[] pub = buildOpenSSHPublicKeyBlob(priv);
        putString(outer, pub);

        // private key blob
        byte[] prv = buildOpenSSHPrivateKeyBlob(priv, comment == null ? "" : comment, rng);
        putString(outer, prv);

        String b64 = Base64.getEncoder().encodeToString(outer.toByteArray());
        StringBuilder pem = new StringBuilder();
        pem.append("-----BEGIN OPENSSH PRIVATE KEY-----\n");
        for (int i = 0; i < b64.length(); i += 70) {
            int end = Math.min(i + 70, b64.length());
            pem.append(b64, i, end).append('\n');
        }
        pem.append("-----END OPENSSH PRIVATE KEY-----");
        return pem.toString();
    }

    private static byte[] buildOpenSSHPublicKeyBlob(RSAPrivateCrtKey priv) throws Exception {
        ByteArrayOutputStream pub = new ByteArrayOutputStream();
        putString(pub, "ssh-rsa");
        putMpint(pub, priv.getPublicExponent());
        putMpint(pub, priv.getModulus());
        return pub.toByteArray();
    }

    private static byte[] buildOpenSSHPrivateKeyBlob(RSAPrivateCrtKey priv, String comment, SecureRandom rng) throws Exception {
        ByteArrayOutputStream prv = new ByteArrayOutputStream();

        int check = rng.nextInt();
        putUint32(prv, check);
        putUint32(prv, check);

        putString(prv, "ssh-rsa");
        putMpint(prv, priv.getModulus());          // n
        putMpint(prv, priv.getPublicExponent());   // e
        putMpint(prv, priv.getPrivateExponent());  // d
        putMpint(prv, priv.getCrtCoefficient());   // iqmp = q^-1 mod p
        putMpint(prv, priv.getPrimeP());           // p
        putMpint(prv, priv.getPrimeQ());           // q
        putString(prv, comment == null ? "" : comment); // comment

        // Padding to a multiple of 8
        byte[] body = prv.toByteArray();
        int block = 8;
        int padLen = block - (body.length % block);
        if (padLen == 0) padLen = block;
        ByteArrayOutputStream padded = new ByteArrayOutputStream();
        padded.write(body);
        for (int i = 1; i <= padLen; i++) {
            padded.write((byte) i);
        }
        return padded.toByteArray();
    }

    private static void putUint32(ByteArrayOutputStream out, long v) throws Exception {
        out.write(new byte[]{
            (byte) ((v >>> 24) & 0xFF),
            (byte) ((v >>> 16) & 0xFF),
            (byte) ((v >>> 8) & 0xFF),
            (byte) (v & 0xFF)
        });
    }

    private static void putString(ByteArrayOutputStream out, String s) throws Exception {
        byte[] b = s.getBytes(StandardCharsets.UTF_8);
        putString(out, b);
    }

    private static void putString(ByteArrayOutputStream out, byte[] b) throws Exception {
        putUint32(out, b.length);
        out.write(b);
    }

    private static void putMpint(ByteArrayOutputStream out, BigInteger bi) throws Exception {
        if (bi.signum() == 0) {
            putUint32(out, 0);
            return;
        }
        byte[] bytes = bi.toByteArray(); // two's complement big-endian, minimal
        // Ensure minimal positive representation (trim redundant leading zeros except one needed for sign)
        int start = 0;
        while (start < bytes.length - 1 && bytes[start] == 0 && (bytes[start + 1] & 0x80) == 0) {
            start++;
        }
        byte[] minimal = new byte[bytes.length - start];
        System.arraycopy(bytes, start, minimal, 0, minimal.length);
        putString(out, minimal);
    }

    // 5 test cases in main
    public static void main(String[] args) throws Exception {
        String k1 = generateOpenSSHRSAPrivateKey(2048, "test1");
        System.out.println(k1);
        System.out.println();

        String k2 = generateOpenSSHRSAPrivateKey(3072, "user@example.com");
        System.out.println(k2);
        System.out.println();

        String k3 = generateOpenSSHRSAPrivateKey(4096, "case3");
        System.out.println(k3);
        System.out.println();

        String k4 = generateOpenSSHRSAPrivateKey(1024, "auto-upgraded-to-2048"); // will be upgraded to 2048
        System.out.println(k4);
        System.out.println();

        String k5 = generateOpenSSHRSAPrivateKey(2048, "");
        System.out.println(k5);
    }
}