import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.*;
import java.util.Base64;

public class Task63 {
    private static final int MAX_INPUT_BYTES = 10 * 1024 * 1024; // 10MB limit

    public static String processRequest(byte[] rawData) throws Exception {
        if (rawData == null) {
            throw new IllegalArgumentException("rawData is null");
        }
        if (rawData.length > MAX_INPUT_BYTES) {
            throw new IllegalArgumentException("Input too large");
        }

        String base64Str = decodeUtf8Strict(rawData);

        // Validate allowed Base64 characters and whitespace only
        for (int i = 0; i < base64Str.length(); i++) {
            char c = base64Str.charAt(i);
            if (!(c >= 'A' && c <= 'Z') &&
                !(c >= 'a' && c <= 'z') &&
                !(c >= '0' && c <= '9') &&
                c != '+' && c != '/' && c != '=' &&
                c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                throw new IllegalArgumentException("Invalid Base64 character detected");
            }
        }

        // Remove whitespace for strict decoding
        String compact = base64Str.replaceAll("\\s+", "");
        byte[] decoded;
        try {
            decoded = Base64.getDecoder().decode(compact);
        } catch (IllegalArgumentException ex) {
            throw new IllegalArgumentException("Base64 decode failed", ex);
        }

        String deserialized = decodeUtf8Strict(decoded);
        String storedData = deserialized;
        return storedData;
    }

    private static String decodeUtf8Strict(byte[] data) throws CharacterCodingException {
        CharsetDecoder dec = StandardCharsets.UTF_8.newDecoder();
        dec.onMalformedInput(CodingErrorAction.REPORT);
        dec.onUnmappableCharacter(CodingErrorAction.REPORT);
        CharBuffer cb = dec.decode(ByteBuffer.wrap(data));
        return cb.toString();
    }

    private static String b64EncodeUtf8(String s) {
        return Base64.getEncoder().encodeToString(s.getBytes(StandardCharsets.UTF_8));
    }

    public static void main(String[] args) {
        String[] originals = new String[] {
            "{\"name\":\"Alice\",\"age\":30}",
            "<person><name>Alice</name><age>30</age></person>",
            "[\"x\",1,true,null]",
            "<root/>",
            "Hello, 世界"
        };
        for (int i = 0; i < originals.length; i++) {
            try {
                String original = originals[i];
                String b64 = b64EncodeUtf8(original);
                String result = processRequest(b64.getBytes(StandardCharsets.UTF_8));
                System.out.println("Test " + (i+1) + " OK: " + result);
            } catch (Exception e) {
                System.out.println("Test " + (i+1) + " Error: " + e.getMessage());
            }
        }
    }
}