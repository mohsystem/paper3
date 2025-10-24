import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Base64;

public class Task78 {

    private static final String KEY_TYPE = "ssh-rsa";
    private static final String CIPHER_NONE = "none";
    private static final String KDF_NONE = "none";

    /**
     * Helper to write a length-prefixed byte array (OpenSSH string format).
     * The length is a 32-bit big-endian integer.
     */
    private static void writeOpenSSHString(DataOutputStream dos, byte[] data) throws IOException {
        dos.writeInt(data.length);
        dos.write(data);
    }

    /**
     * Helper to write a BigInteger (OpenSSH mpint format).
     * RSA components are positive, but if the most significant bit is 1,
     * BigInteger's toByteArray() prepends a 0x00 to keep it positive,
     * which is the correct representation for an mpint.
     */
    private static void writeMpint(DataOutputStream dos, BigInteger bi) throws IOException {
        writeOpenSSHString(dos, bi.toByteArray());
    }

    /**
     * Generates an RSA private key and exports it using the OpenSSH format.
     *
     * @param keySize The size of the key in bits (e.g., 2048, 4096).
     * @return The RSA private key in OpenSSH PEM format.
     */
    public static String generateOpenSSHPrivateKey(int keySize) throws NoSuchAlgorithmException, IOException {
        // 1. Generate an RSA Key Pair
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(keySize, new SecureRandom());
        KeyPair pair = keyGen.generateKeyPair();

        RSAPublicKey pub = (RSAPublicKey) pair.getPublic();
        RSAPrivateCrtKey priv = (RSAPrivateCrtKey) pair.getPrivate();

        // 2. Create the public key blob
        ByteArrayOutputStream publicKeyBaos = new ByteArrayOutputStream();
        DataOutputStream publicKeyDos = new DataOutputStream(publicKeyBaos);
        writeOpenSSHString(publicKeyDos, KEY_TYPE.getBytes());
        writeMpint(publicKeyDos, pub.getPublicExponent());
        writeMpint(publicKeyDos, pub.getModulus());
        publicKeyDos.close();

        // 3. Create the private key blob
        ByteArrayOutputStream privateKeyBaos = new ByteArrayOutputStream();
        DataOutputStream privateKeyDos = new DataOutputStream(privateKeyBaos);

        // A random 32-bit integer, written twice for a check
        int checkInt = new SecureRandom().nextInt();
        privateKeyDos.writeInt(checkInt);
        privateKeyDos.writeInt(checkInt);

        writeOpenSSHString(privateKeyDos, KEY_TYPE.getBytes());
        writeMpint(privateKeyDos, priv.getModulus());          // n
        writeMpint(privateKeyDos, priv.getPublicExponent());   // e
        writeMpint(privateKeyDos, priv.getPrivateExponent());  // d
        writeMpint(privateKeyDos, priv.getCrtCoefficient());   // iqmp (q^-1 mod p)
        writeMpint(privateKeyDos, priv.getPrimeP());           // p
        writeMpint(privateKeyDos, priv.getPrimeQ());           // q

        // comment (empty)
        writeOpenSSHString(privateKeyDos, "".getBytes());

        // padding
        int blockSize = 8;
        int currentLength = privateKeyBaos.size();
        int paddingLength = (blockSize - (currentLength % blockSize)) % blockSize;
        for (int i = 0; i < paddingLength; i++) {
            privateKeyDos.writeByte(i + 1);
        }
        privateKeyDos.close();

        // 4. Assemble the final OpenSSH key structure
        ByteArrayOutputStream finalKeyBaos = new ByteArrayOutputStream();
        DataOutputStream finalKeyDos = new DataOutputStream(finalKeyBaos);

        finalKeyDos.write("openssh-key-v1\0".getBytes()); // Magic phrase
        writeOpenSSHString(finalKeyDos, CIPHER_NONE.getBytes());
        writeOpenSSHString(finalKeyDos, KDF_NONE.getBytes());
        writeOpenSSHString(finalKeyDos, new byte[0]); // kdfoptions
        finalKeyDos.writeInt(1); // number of keys

        writeOpenSSHString(finalKeyDos, publicKeyBaos.toByteArray());
        writeOpenSSHString(finalKeyDos, privateKeyBaos.toByteArray());
        finalKeyDos.close();

        // 5. Base64 encode and wrap in PEM headers
        String base64Key = Base64.getEncoder().encodeToString(finalKeyBaos.toByteArray());
        StringBuilder result = new StringBuilder();
        result.append("-----BEGIN OPENSSH PRIVATE KEY-----\n");
        int offset = 0;
        while (offset < base64Key.length()) {
            int end = Math.min(offset + 70, base64Key.length());
            result.append(base64Key.substring(offset, end)).append("\n");
            offset = end;
        }
        result.append("-----END OPENSSH PRIVATE KEY-----\n");

        return result.toString();
    }

    public static void main(String[] args) {
        int[] testKeySizes = {1024, 1024, 2048, 2048, 4096};
        for (int i = 0; i < testKeySizes.length; i++) {
            try {
                System.out.println("--- Test Case " + (i + 1) + ": Generating " + testKeySizes[i] + "-bit key ---");
                String key = generateOpenSSHPrivateKey(testKeySizes[i]);
                System.out.println(key);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}