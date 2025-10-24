import java.io.IOException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.PrivateKey;
import java.security.Security;
import java.util.Base64;
import org.bouncycastle.crypto.util.OpenSSHPrivateKeyUtil;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.openssl.jcajce.JcaPEMKeyConverter;

/**
 * Note: This code requires the Bouncy Castle library.
 * You need to add the 'bcpkix' and 'bcprov' JARs to your project's classpath.
 * For example, using Maven:
 * <dependency>
 *     <groupId>org.bouncycastle</groupId>
 *     <artifactId>bcpkix-jdk18on</artifactId>
 *     <version>1.78.1</version>
 * </dependency>
 * <dependency>
 *     <groupId>org.bouncycastle</groupId>
 *     <artifactId>bcprov-jdk18on</artifactId>
 *     <version>1.78.1</version>
 * </dependency>
 */
public class Task78 {

    /**
     * Generates an RSA private key and returns it in the OpenSSH format.
     *
     * @param keySize The size of the key in bits (e.g., 2048, 4096).
     * @return A string containing the RSA private key in OpenSSH format, or null on failure.
     */
    public static String generateOpenSSHPrivateKey(int keySize) {
        // Add Bouncy Castle as a security provider if it's not already present
        if (Security.getProvider(BouncyCastleProvider.PROVIDER_NAME) == null) {
            Security.addProvider(new BouncyCastleProvider());
        }

        try {
            // Generate an RSA key pair using the Bouncy Castle provider
            KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA", "BC");
            keyPairGenerator.initialize(keySize);
            KeyPair keyPair = keyPairGenerator.generateKeyPair();
            PrivateKey privateKey = keyPair.getPrivate();

            // Use Bouncy Castle utilities to convert the JCA private key to the OpenSSH format
            byte[] openSshPrivateKeyBytes = OpenSSHPrivateKeyUtil.encodePrivateKey(
                new JcaPEMKeyConverter().getPrivateKeyParameter(privateKey)
            );

            // The OpenSSH format is wrapped in a PEM-like structure
            String header = "-----BEGIN OPENSSH PRIVATE KEY-----\n";
            String footer = "\n-----END OPENSSH PRIVATE KEY-----";
            String base64Key = Base64.getEncoder().encodeToString(openSshPrivateKeyBytes);

            // Format with line breaks every 64 characters for PEM standard compliance
            StringBuilder formattedKey = new StringBuilder();
            int i = 0;
            while (i < base64Key.length()) {
                formattedKey.append(base64Key.substring(i, Math.min(i + 64, base64Key.length())));
                formattedKey.append("\n");
                i += 64;
            }
            
            // Trim the final newline before adding the footer
            return header + formattedKey.toString().trim() + footer;

        } catch (NoSuchAlgorithmException | NoSuchProviderException | IOException e) {
            System.err.println("An error occurred during key generation: " + e.getMessage());
            e.printStackTrace();
            return null;
        } catch (NoClassDefFoundError e) {
             // This error occurs if the Bouncy Castle JARs are not in the classpath
            System.err.println("Error: Bouncy Castle library not found. " +
                               "Please add bcpkix and bcprov JARs to your classpath.");
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java RSA OpenSSH Key Generation ---");
        System.out.println("Note: Requires the Bouncy Castle library.");
        
        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " (2048 bits) ---");
            String privateKey = generateOpenSSHPrivateKey(2048);
            if (privateKey != null) {
                System.out.println(privateKey);
            } else {
                System.out.println("Key generation failed.");
            }
        }
    }
}