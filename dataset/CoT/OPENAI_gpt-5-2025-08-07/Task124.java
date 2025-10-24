import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.AEADBadTagException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/*
Chain-of-Through process in code comments:
1) Problem understanding:
   - Purpose: Securely store sensitive data (e.g., credit card numbers and personal info) using strong encryption,
     with safe input validation, minimal exposure, and secure memory handling where feasible.
   - Inputs: identifier, user name (non-sensitive), credit card number (sensitive), passphrase for key derivation.
   - Operations: Luhn validation for card, key derivation (PBKDF2), authenticated encryption (AES-GCM),
     in-memory storage of encrypted records, retrieval with passphrase, masked display only.
   - Outputs: boolean success, masked data strings, and retrieval on correct passphrase.

2) Security requirements:
   - Strong cryptography: PBKDF2WithHmacSHA256 with high iteration count and random salt; AES-256-GCM with random IV.
   - Authenticated encryption: prevents tampering; wrong passphrase or modified data -> decryption fails.
   - No logging/printing sensitive plaintext; only masked or metadata.
   - Minimal lifetime of sensitive data in memory; wipe temporary byte arrays whenever possible.
   - Input validation of credit card numbers with Luhn check.
   - Unique salt and IV per record.

3) Secure coding generation:
   - Use SecureRandom for salt/IV.
   - Use char[] for passphrase, convert to bytes without creating extra Strings; wipe derived byte arrays.
   - Enforce GCM tag size 128 bits.
   - Keep only ciphertext, salt, IV in memory store; never store plaintext or keys.

4) Code review (embedded as comments during development):
   - Avoid String for secrets; wipe byte arrays.
   - Catch AEADBadTagException and do not leak info.
   - Validate inputs; reject invalid card numbers.
   - Do not expose decrypted data via logs; only masked.

5) Secure code output:
   - Final code integrates mitigations and adheres to secure patterns.
*/

public class Task124 {

    // Secure in-memory store: ID -> EncryptedRecord (salt, iv, ciphertext)
    private final Map<String, EncryptedRecord> store = new HashMap<>();

    private static final SecureRandom RNG = new SecureRandom();
    private static final int SALT_LEN = 16;   // 128-bit salt
    private static final int IV_LEN = 12;     // 96-bit IV for AES-GCM
    private static final int KEY_LEN = 32;    // 256-bit key
    private static final int GCM_TAG_BITS = 128;
    private static final int PBKDF2_ITERS = 200_000;

    // Container for encrypted data
    private static final class EncryptedRecord {
        final byte[] salt;
        final byte[] iv;
        final byte[] ciphertext;
        EncryptedRecord(byte[] salt, byte[] iv, byte[] ciphertext) {
            this.salt = salt;
            this.iv = iv;
            this.ciphertext = ciphertext;
        }
    }

    // Plain record (avoid exposing; used internally)
    private static final class SensitiveRecord {
        final String name; // non-sensitive for demo
        final String card; // sensitive, avoid printing full
        SensitiveRecord(String name, String card) {
            this.name = name;
            this.card = card;
        }
    }

    // Public API: add a record (returns true if stored)
    public boolean addRecord(String id, char[] passphrase, String name, char[] cardNumber) {
        if (id == null || id.isEmpty() || name == null) return false;
        if (cardNumber == null || cardNumber.length == 0) return false;

        // Validate card via Luhn
        if (!isValidCardLuhn(cardNumber)) {
            wipeChars(cardNumber);
            return false;
        }

        // Serialize minimal plaintext: "name\ncard"
        byte[] plaintext = null;
        try {
            plaintext = serialize(name, cardNumber);

            // Encrypt
            EncryptedRecord rec = encrypt(passphrase, plaintext);
            if (rec == null) return false;

            // Store encrypted record only
            synchronized (store) {
                store.put(id, rec);
            }
            return true;
        } finally {
            // Wipe sensitive buffers
            if (plaintext != null) wipeBytes(plaintext);
            wipeChars(cardNumber);
            // Do not wipe passphrase here; caller may reuse; caller can wipe after use
        }
    }

    // Public API: get masked view "Name: X, Card: **** **** **** 1234"
    public String getMaskedRecord(String id, char[] passphrase) {
        EncryptedRecord rec;
        synchronized (store) {
            rec = store.get(id);
        }
        if (rec == null) return null;

        byte[] plaintext = null;
        try {
            plaintext = decrypt(passphrase, rec);
            if (plaintext == null) return null;
            SensitiveRecord sr = deserialize(plaintext);
            if (sr == null) return null;
            return "Name: " + sr.name + ", Card: " + maskCard(sr.card);
        } catch (AEADBadTagException e) {
            // Wrong passphrase or tampered data
            return null;
        } catch (Exception e) {
            return null;
        } finally {
            if (plaintext != null) wipeBytes(plaintext);
        }
    }

    // Public API: delete a record securely
    public boolean deleteRecord(String id) {
        EncryptedRecord rec;
        synchronized (store) {
            rec = store.remove(id);
        }
        if (rec == null) return false;
        // Wipe encrypted blobs (even though ciphertext, still good hygiene)
        wipeBytes(rec.salt);
        wipeBytes(rec.iv);
        wipeBytes(rec.ciphertext);
        return true;
    }

    // Encryption using AES-256-GCM, key derived with PBKDF2(HMAC-SHA256)
    private static EncryptedRecord encrypt(char[] passphrase, byte[] plaintext) {
        byte[] salt = new byte[SALT_LEN];
        byte[] iv = new byte[IV_LEN];
        RNG.nextBytes(salt);
        RNG.nextBytes(iv);

        byte[] keyBytes = null;
        try {
            keyBytes = deriveKey(passphrase, salt);
            SecretKeySpec key = new SecretKeySpec(keyBytes, "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcm = new GCMParameterSpec(GCM_TAG_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, gcm);
            byte[] ciphertext = cipher.doFinal(plaintext);
            return new EncryptedRecord(salt, iv, ciphertext);
        } catch (Exception e) {
            return null;
        } finally {
            if (keyBytes != null) wipeBytes(keyBytes);
        }
    }

    // Decryption
    private static byte[] decrypt(char[] passphrase, EncryptedRecord rec) throws Exception {
        byte[] keyBytes = null;
        try {
            keyBytes = deriveKey(passphrase, rec.salt);
            SecretKeySpec key = new SecretKeySpec(keyBytes, "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcm = new GCMParameterSpec(GCM_TAG_BITS, rec.iv);
            cipher.init(Cipher.DECRYPT_MODE, key, gcm);
            return cipher.doFinal(rec.ciphertext);
        } finally {
            if (keyBytes != null) wipeBytes(keyBytes);
        }
    }

    // PBKDF2 key derivation
    private static byte[] deriveKey(char[] passphrase, byte[] salt) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(passphrase, salt, PBKDF2_ITERS, KEY_LEN * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] key = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return key;
    }

    // Serialize: "name\ncard"
    private static byte[] serialize(String name, char[] card) {
        // name considered less sensitive; card is sensitive
        byte[] nameBytes = null;
        byte[] cardBytes = null;
        byte[] out = null;
        try {
            nameBytes = name.getBytes(StandardCharsets.UTF_8);
            cardBytes = charsToBytes(card, StandardCharsets.UTF_8);
            out = new byte[nameBytes.length + 1 + cardBytes.length];
            System.arraycopy(nameBytes, 0, out, 0, nameBytes.length);
            out[nameBytes.length] = (byte) '\n';
            System.arraycopy(cardBytes, 0, out, nameBytes.length + 1, cardBytes.length);
            return out;
        } finally {
            if (nameBytes != null) wipeBytes(nameBytes); // hygiene
            if (cardBytes != null) wipeBytes(cardBytes);
            // do not wipe 'card' here; caller wipes
        }
    }

    // Deserialize: split at first '\n'
    private static SensitiveRecord deserialize(byte[] data) {
        int idx = -1;
        for (int i = 0; i < data.length; i++) {
            if (data[i] == (byte) '\n') { idx = i; break; }
        }
        if (idx < 0) return null;
        String name = new String(data, 0, idx, StandardCharsets.UTF_8);
        String card = new String(data, idx + 1, data.length - (idx + 1), StandardCharsets.UTF_8);
        return new SensitiveRecord(name, card);
    }

    // Luhn check
    private static boolean isValidCardLuhn(char[] digits) {
        int sum = 0;
        boolean dbl = false;
        for (int i = digits.length - 1; i >= 0; i--) {
            char c = digits[i];
            if (c < '0' || c > '9') {
                if (c == ' ' || c == '-') continue; // allow separators
                return false;
            }
            int n = c - '0';
            if (dbl) {
                n *= 2;
                if (n > 9) n -= 9;
            }
            sum += n;
            dbl = !dbl;
        }
        return sum % 10 == 0;
    }

    private static String maskCard(String card) {
        // Keep last 4, mask others; preserve length grouping by 4 if digits
        String digitsOnly = card.replaceAll("[^0-9]", "");
        int len = digitsOnly.length();
        if (len <= 4) return digitsOnly;
        String last4 = digitsOnly.substring(len - 4);
        int maskedCount = len - 4;
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < maskedCount; i++) {
            sb.append('*');
            if ((i + 1) % 4 == 0) sb.append(' ');
        }
        if (sb.length() > 0 && sb.charAt(sb.length() - 1) != ' ') sb.append(' ');
        // Append last 4 with spacing
        for (int i = 0; i < last4.length(); i++) {
            sb.append(last4.charAt(i));
            if ((maskedCount + i + 1) % 4 == 0 && i != last4.length() - 1) sb.append(' ');
        }
        return sb.toString().trim();
    }

    // Helper: convert char[] to bytes without creating String
    private static byte[] charsToBytes(char[] chars, Charset cs) {
        CharBuffer cb = CharBuffer.wrap(chars);
        ByteBuffer bb = cs.encode(cb);
        byte[] out = new byte[bb.remaining()];
        bb.get(out);
        // wipe temporary buffers
        wipeCharBuffer(cb);
        wipeByteBuffer(bb);
        return out;
    }

    private static void wipeBytes(byte[] b) {
        if (b != null) Arrays.fill(b, (byte) 0);
    }

    private static void wipeChars(char[] c) {
        if (c != null) Arrays.fill(c, '\0');
    }

    private static void wipeCharBuffer(CharBuffer cb) {
        if (cb != null && cb.hasArray()) {
            char[] arr = cb.array();
            Arrays.fill(arr, '\0');
        }
    }

    private static void wipeByteBuffer(ByteBuffer bb) {
        if (bb != null && bb.hasArray()) {
            byte[] arr = bb.array();
            Arrays.fill(arr, (byte) 0);
        }
    }

    // Demo main with 5 test cases
    public static void main(String[] args) {
        Task124 svc = new Task124();

        // Passphrase used to derive encryption keys
        char[] pass = "Strong-Passphrase_OnlyForDemo!".toCharArray();
        char[] passWrong = "Wrong-Passphrase".toCharArray();

        // Test cases (valid Luhn test numbers)
        String[] ids = { "rec1", "rec2", "rec3", "rec4", "rec5" };
        String[] names = { "Alice", "Bob", "Carol", "Dave", "Eve" };
        String[] cards = {
            "4111 1111 1111 1111",   // Visa
            "5555-5555-5555-4444",   // MasterCard
            "378282246310005",       // Amex
            "6011111111111117",      // Discover
            "4012888888881881"       // Visa
        };

        // Add records
        for (int i = 0; i < ids.length; i++) {
            boolean ok = svc.addRecord(ids[i], pass, names[i], cards[i].toCharArray());
            System.out.println("Add " + ids[i] + " -> " + (ok ? "OK" : "FAIL"));
        }

        // Retrieve masked with correct passphrase
        for (String id : ids) {
            String masked = svc.getMaskedRecord(id, pass);
            System.out.println("Masked " + id + " -> " + (masked != null ? masked : "ACCESS DENIED"));
        }

        // Attempt with wrong passphrase (should fail)
        for (String id : ids) {
            String masked = svc.getMaskedRecord(id, passWrong);
            System.out.println("Masked with wrong pass " + id + " -> " + (masked != null ? masked : "ACCESS DENIED"));
        }

        // Delete records securely
        for (String id : ids) {
            boolean del = svc.deleteRecord(id);
            System.out.println("Delete " + id + " -> " + (del ? "OK" : "FAIL"));
        }

        // Wipe passphrases
        wipeChars(pass);
        wipeChars(passWrong);
    }
}