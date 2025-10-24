import java.nio.charset.StandardCharsets;

public class Task79 {
    public static String encrypt(String message, String key) {
        byte[] mb = message.getBytes(StandardCharsets.UTF_8);
        byte[] kb = key.getBytes(StandardCharsets.UTF_8);
        if (kb.length == 0) throw new IllegalArgumentException("Key must not be empty");
        byte[] out = new byte[mb.length];
        for (int i = 0; i < mb.length; i++) {
            out[i] = (byte) (mb[i] ^ kb[i % kb.length]);
        }
        StringBuilder sb = new StringBuilder(out.length * 2);
        for (byte b : out) {
            sb.append(String.format("%02X", b));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println(encrypt("HELLO WORLD", "KEY"));
        System.out.println(encrypt("Attack at dawn!", "secret"));
        System.out.println(encrypt("OpenAI", "GPT"));
        System.out.println(encrypt("The quick brown fox jumps over the lazy dog.", "cipher"));
        System.out.println(encrypt("Lorem ipsum dolor sit amet", "xyz"));
    }
}