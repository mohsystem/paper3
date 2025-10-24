// NOTE: This code requires the Bouncy Castle library.
// To compile and run, you need to have the bcprov and bcpkix JARs in your classpath.
// For example, using Maven, add these dependencies to your pom.xml:
// <dependencies>
//     <dependency>
//         <groupId>org.bouncycastle</groupId>
//         <artifactId>bcprov-jdk18on</artifactId>
//         <version>1.78.1</version>
//     </dependency>
//     <dependency>
//         <groupId>org.bouncycastle</groupId>
//         <artifactId>bcpkix-jdk18on</artifactId>
//         <version>1.78.1</version>
//     </dependency>
// </dependencies>

import java.io.IOException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.Security;
import java.util.Base64;
import org.bouncycastle.crypto.params.AsymmetricKeyParameter;
import org.bouncycastle.crypto.util.OpenSSHPrivateKeyUtil;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.openssl.jcajce.JcaPEMKeyConverter;

public class Task78 {

    /**
     * Generates an RSA private key and exports it in the OpenSSH format.
     *
     * @param keySize The key size in bits. Must be 2048 or greater.
     * @return The RSA private key as a string in OpenSSH format.
     * @throws NoSuchAlgorithmException if the RSA algorithm is not available.
     * @throws IOException if an error occurs during key encoding.
     */
    public static String generateOpenSSHPrivateKey(int keySize) throws NoSuchAlgorithmException, IOException {
        if (keySize < 2048) {
            throw new IllegalArgumentException("Key size must be at least 2048 bits.");
        }

        if (Security.getProvider(BouncyCastleProvider.PROVIDER_NAME) == null) {
            Security.addProvider(new BouncyCastleProvider());
        }

        KeyPairGenerator generator = KeyPairGenerator.getInstance("RSA");
        generator.initialize(keySize, new SecureRandom());
        KeyPair keyPair = generator.generateKeyPair();

        AsymmetricKeyParameter bcPrivateKey = new JcaPEMKeyConverter().getPrivateKey(keyPair.getPrivate());

        byte[] opensshPrivateKeyBytes = OpenSSHPrivateKeyUtil.encodePrivateKey(bcPrivateKey);

        String base64Key = Base64.getEncoder().encodeToString(opensshPrivateKeyBytes);

        StringBuilder result = new StringBuilder();
        result.append("-----BEGIN OPENSSH PRIVATE KEY-----\n");
        int i = 0;
        while (i < base64Key.length()) {
            int end = Math.min(i + 64, base64Key.length());
            result.append(base64Key.substring(i, end)).append("\n");
            i = end;
        }
        result.append("-----END OPENSSH PRIVATE KEY-----\n");

        return result.toString();
    }

    public static void main(String[] args) {
        int[] keySizes = {2048, 2048, 3072, 4096, 4096};
        for (int i = 0; i < keySizes.length; i++) {
            try {
                System.out.println("--- Test Case " + (i + 1) + " (Key Size: " + keySizes[i] + ") ---");
                String privateKey = generateOpenSSHPrivateKey(keySizes[i]);
                System.out.println(privateKey);
            } catch (NoSuchAlgorithmException | IOException | IllegalArgumentException e) {
                System.err.println("Error generating key for size " + keySizes[i] + ": " + e.getMessage());
            }
        }
    }
}