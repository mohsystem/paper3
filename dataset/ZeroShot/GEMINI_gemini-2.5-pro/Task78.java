/*
NOTE: This Java code requires the Bouncy Castle library.
Please add the following dependencies to your project's build file.
For Maven:
<dependency>
    <groupId>org.bouncycastle</groupId>
    <artifactId>bcprov-jdk18on</artifactId>
    <version>1.78</version>
</dependency>
<dependency>
    <groupId>org.bouncycastle</groupId>
    <artifactId>bcpkix-jdk18on</artifactId>
    <version>1.78</version>
</dependency>

For Gradle:
implementation 'org.bouncycastle:bcprov-jdk18on:1.78'
implementation 'org.bouncycastle:bcpkix-jdk18on:1.78'
*/
import java.io.StringWriter;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.Security;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.openssl.jcajce.JcaMiscPEMGenerator;
import org.bouncycastle.util.io.pem.PemWriter;

public class Task78 {

    static {
        // Add Bouncy Castle as a security provider
        if (Security.getProvider(BouncyCastleProvider.PROVIDER_NAME) == null) {
            Security.addProvider(new BouncyCastleProvider());
        }
    }

    /**
     * Generates an RSA private key and exports it in the OpenSSH format.
     *
     * @param keySize The size of the key in bits (e.g., 2048, 4096).
     * @return The RSA private key in OpenSSH PEM format, or null on error.
     */
    public static String generateAndExportRsaKey(int keySize) {
        if (keySize < 2048) {
            System.err.println("Warning: Key sizes smaller than 2048 are not recommended.");
        }

        try {
            // 1. Generate RSA Key Pair
            KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA", "BC");
            keyGen.initialize(keySize);
            KeyPair keyPair = keyGen.generateKeyPair();

            // 2. Use Bouncy Castle's PEM generator for OpenSSH format
            StringWriter stringWriter = new StringWriter();
            try (PemWriter pemWriter = new PemWriter(stringWriter)) {
                pemWriter.writeObject(new JcaMiscPEMGenerator(keyPair.getPrivate()));
            }

            return stringWriter.toString();

        } catch (NoSuchAlgorithmException e) {
            // This should not happen if RSA is supported by the provider
            System.err.println("RSA algorithm not found: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An error occurred during key generation or export: " + e.getMessage());
        }
        return null;
    }

    public static void main(String[] args) {
        // 5 Test Cases
        int[] keySizes = {2048, 2048, 3072, 4096, 4096};
        for (int i = 0; i < keySizes.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " (Key Size: " + keySizes[i] + ") ---");
            String openSshKey = generateAndExportRsaKey(keySizes[i]);
            if (openSshKey != null) {
                System.out.println(openSshKey);
            } else {
                System.out.println("Failed to generate key.");
            }
        }
    }
}