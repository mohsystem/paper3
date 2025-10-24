
import javax.crypto.Cipher;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.spec.RSAKeyGenParameterSpec;
import java.util.Base64;

public class Task78 {
    private static final int KEY_SIZE = 2048;
    private static final String KEY_TYPE = "ssh-rsa";
    
    public static String generateAndExportRSAKey() throws NoSuchAlgorithmException, InvalidAlgorithmParameterException, IOException {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(new RSAKeyGenParameterSpec(KEY_SIZE, RSAKeyGenParameterSpec.F4), new SecureRandom());
        KeyPair keyPair = keyGen.generateKeyPair();
        
        RSAPrivateCrtKey privateKey = (RSAPrivateCrtKey) keyPair.getPrivate();
        
        return exportToOpenSSHFormat(privateKey);
    }
    
    private static String exportToOpenSSHFormat(RSAPrivateCrtKey privateKey) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        
        writeString(baos, "ssh-rsa");
        writeMPInt(baos, privateKey.getModulus());
        writeMPInt(baos, privateKey.getPublicExponent());
        writeMPInt(baos, privateKey.getPrivateExponent());
        writeMPInt(baos, privateKey.getCrtCoefficient());
        writeMPInt(baos, privateKey.getPrimeP());
        writeMPInt(baos, privateKey.getPrimeQ());
        
        byte[] encoded = baos.toByteArray();
        String base64Encoded = Base64.getEncoder().encodeToString(encoded);
        
        StringBuilder result = new StringBuilder();
        result.append("-----BEGIN OPENSSH PRIVATE KEY-----\\n");
        
        int lineLength = 70;
        for (int i = 0; i < base64Encoded.length(); i += lineLength) {
            int end = Math.min(i + lineLength, base64Encoded.length());
            result.append(base64Encoded, i, end);
            result.append("\\n");
        }
        
        result.append("-----END OPENSSH PRIVATE KEY-----\\n");
        
        return result.toString();
    }
    
    private static void writeString(ByteArrayOutputStream baos, String str) throws IOException {
        byte[] bytes = str.getBytes(StandardCharsets.UTF_8);
        writeInt(baos, bytes.length);
        baos.write(bytes);
    }
    
    private static void writeMPInt(ByteArrayOutputStream baos, BigInteger value) throws IOException {
        byte[] bytes = value.toByteArray();
        writeInt(baos, bytes.length);
        baos.write(bytes);
    }
    
    private static void writeInt(ByteArrayOutputStream baos, int value) throws IOException {
        baos.write((value >>> 24) & 0xFF);
        baos.write((value >>> 16) & 0xFF);
        baos.write((value >>> 8) & 0xFF);
        baos.write(value & 0xFF);
    }
    
    public static void main(String[] args) {
        try {
            for (int i = 1; i <= 5; i++) {
                System.out.println("Test Case " + i + ":");
                String privateKey = generateAndExportRSAKey();
                System.out.println(privateKey.substring(0, Math.min(200, privateKey.length())) + "...");
                System.out.println("Key generated successfully\\n");
            }
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
