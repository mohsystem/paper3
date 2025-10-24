import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Objects;
import java.util.regex.Pattern;

public class Task131 {

    public static final class UserRecord {
        public final long userId; // 0..4294967295
        public final String username; // ASCII [A-Za-z0-9_.-]{1,64}
        public final int age; // 0..120
        public final String email; // Simplified validation, max 254

        public UserRecord(long userId, String username, int age, String email) {
            this.userId = userId;
            this.username = username;
            this.age = age;
            this.email = email;
        }

        @Override
        public String toString() {
            return "UserRecord{id=" + userId + ", username='" + username + "', age=" + age + ", email='" + email + "'}";
        }
    }

    // Constants
    private static final byte[] MAGIC = new byte[] { 'U', 'S', 'R', '1' };
    private static final int VERSION = 1;
    private static final int USERNAME_MIN = 1;
    private static final int USERNAME_MAX = 64;
    private static final int EMAIL_MIN = 3;
    private static final int EMAIL_MAX = 254;
    private static final int AGE_MIN = 0;
    private static final int AGE_MAX = 120;

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{1,64}$");
    // Simplified but safe email pattern
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");

    // Public API: Deserialize user-supplied data into a UserRecord
    public static UserRecord deserialize(byte[] data) {
        Objects.requireNonNull(data, "data");
        int pos = 0;

        if (data.length < 12) {
            throw new IllegalArgumentException("Data too short");
        }
        // MAGIC
        if (!Arrays.equals(Arrays.copyOfRange(data, 0, 4), MAGIC)) {
            throw new IllegalArgumentException("Invalid magic");
        }
        pos += 4;

        // VERSION
        int version = data[pos++] & 0xFF;
        if (version != VERSION) {
            throw new IllegalArgumentException("Unsupported version");
        }

        // userId (uint32 big-endian)
        long userId = ((long) (data[pos++] & 0xFF) << 24)
                    | ((long) (data[pos++] & 0xFF) << 16)
                    | ((long) (data[pos++] & 0xFF) << 8)
                    | ((long) (data[pos++] & 0xFF));

        // username length
        int uLen = data[pos++] & 0xFF;
        if (uLen < USERNAME_MIN || uLen > USERNAME_MAX) {
            throw new IllegalArgumentException("Invalid username length");
        }
        ensureRemaining(data.length, pos, uLen, "username");

        String username = decodeAsciiUtf8Strict(data, pos, uLen);
        pos += uLen;
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            throw new IllegalArgumentException("Invalid username content");
        }

        // age
        int age = data[pos++] & 0xFF;
        if (age < AGE_MIN || age > AGE_MAX) {
            throw new IllegalArgumentException("Invalid age");
        }

        // email length
        int eLen = data[pos++] & 0xFF;
        if (eLen < EMAIL_MIN || eLen > EMAIL_MAX) {
            throw new IllegalArgumentException("Invalid email length");
        }
        ensureRemaining(data.length, pos, eLen, "email");

        String email = decodeAsciiUtf8Strict(data, pos, eLen);
        pos += eLen;
        if (email.length() > EMAIL_MAX || !EMAIL_PATTERN.matcher(email).matches()) {
            throw new IllegalArgumentException("Invalid email content");
        }

        // Entire buffer should be exactly consumed
        if (pos != data.length) {
            throw new IllegalArgumentException("Trailing data present");
        }

        return new UserRecord(userId, username, age, email);
    }

    // Helper to serialize (safe) for testing
    public static byte[] serialize(UserRecord r) {
        Objects.requireNonNull(r, "record");
        if (r.userId < 0 || r.userId > 0xFFFFFFFFL) {
            throw new IllegalArgumentException("userId out of range");
        }
        if (r.username == null || r.username.length() < USERNAME_MIN || r.username.length() > USERNAME_MAX) {
            throw new IllegalArgumentException("username length invalid");
        }
        if (!USERNAME_PATTERN.matcher(r.username).matches()) {
            throw new IllegalArgumentException("username content invalid");
        }
        if (r.age < AGE_MIN || r.age > AGE_MAX) {
            throw new IllegalArgumentException("age out of range");
        }
        if (r.email == null) {
            throw new IllegalArgumentException("email null");
        }
        byte[] uBytes = r.username.getBytes(StandardCharsets.UTF_8);
        byte[] eBytes = r.email.getBytes(StandardCharsets.UTF_8);
        if (!isAscii(uBytes) || !isAscii(eBytes)) {
            throw new IllegalArgumentException("Only ASCII is permitted");
        }
        if (uBytes.length < USERNAME_MIN || uBytes.length > USERNAME_MAX) {
            throw new IllegalArgumentException("username encoded length invalid");
        }
        if (eBytes.length < EMAIL_MIN || eBytes.length > EMAIL_MAX) {
            throw new IllegalArgumentException("email length invalid");
        }
        if (!EMAIL_PATTERN.matcher(r.email).matches()) {
            throw new IllegalArgumentException("email content invalid");
        }

        int total = 4 + 1 + 4 + 1 + uBytes.length + 1 + 1 + eBytes.length;
        byte[] out = new byte[total];
        int pos = 0;
        // magic
        System.arraycopy(MAGIC, 0, out, pos, MAGIC.length);
        pos += MAGIC.length;
        // version
        out[pos++] = (byte) VERSION;
        // userId
        out[pos++] = (byte) ((r.userId >>> 24) & 0xFF);
        out[pos++] = (byte) ((r.userId >>> 16) & 0xFF);
        out[pos++] = (byte) ((r.userId >>> 8) & 0xFF);
        out[pos++] = (byte) (r.userId & 0xFF);
        // username
        out[pos++] = (byte) (uBytes.length & 0xFF);
        System.arraycopy(uBytes, 0, out, pos, uBytes.length);
        pos += uBytes.length;
        // age
        out[pos++] = (byte) (r.age & 0xFF);
        // email
        out[pos++] = (byte) (eBytes.length & 0xFF);
        System.arraycopy(eBytes, 0, out, pos, eBytes.length);
        pos += eBytes.length;
        return out;
    }

    private static void ensureRemaining(int total, int pos, int needed, String field) {
        if (needed < 0 || pos < 0 || pos > total || total - pos < needed) {
            throw new IllegalArgumentException("Insufficient data for " + field);
        }
    }

    private static String decodeAsciiUtf8Strict(byte[] data, int offset, int len) {
        byte[] slice = Arrays.copyOfRange(data, offset, offset + len);
        if (!isAscii(slice)) {
            throw new IllegalArgumentException("Non-ASCII characters not allowed");
        }
        return new String(slice, StandardCharsets.UTF_8);
    }

    private static boolean isAscii(byte[] arr) {
        for (byte b : arr) {
            int v = b & 0xFF;
            if (v < 0x20 || v > 0x7E) { // printable ASCII only
                return false;
            }
        }
        return true;
    }

    // Test harness with 5 test cases
    public static void main(String[] args) {
        // 1) Valid
        UserRecord u1 = new UserRecord(123456L, "alice_01", 30, "alice@example.com");
        byte[] d1 = serialize(u1);
        System.out.println("T1: " + deserialize(d1));

        // 2) Invalid magic
        byte[] badMagic = Arrays.copyOf(d1, d1.length);
        badMagic[0] = 'B';
        try {
            deserialize(badMagic);
            System.out.println("T2: Unexpected success");
        } catch (IllegalArgumentException e) {
            System.out.println("T2: " + e.getMessage());
        }

        // 3) Invalid age (set to 200)
        byte[] badAge = Arrays.copyOf(d1, d1.length);
        // age byte is at position: 4 magic +1 ver +4 id +1 ulen + ulen
        int agePos = 4 + 1 + 4 + 1 + "alice_01".getBytes(StandardCharsets.UTF_8).length;
        badAge[agePos] = (byte) 200;
        try {
            deserialize(badAge);
            System.out.println("T3: Unexpected success");
        } catch (IllegalArgumentException e) {
            System.out.println("T3: " + e.getMessage());
        }

        // 4) Invalid username content (contains space)
        UserRecord u4 = new UserRecord(42L, "bad name", 25, "bn@example.org");
        try {
            byte[] d4 = serialize(u4); // should fail at serialize
            System.out.println("T4: Unexpected success serialize length=" + d4.length);
        } catch (IllegalArgumentException e) {
            System.out.println("T4: " + e.getMessage());
        }

        // 5) Invalid email content (missing '@')
        UserRecord u5 = new UserRecord(77L, "bob-77", 44, "bob#example.com");
        try {
            byte[] d5 = serialize(u5); // should fail
            System.out.println("T5: Unexpected success serialize length=" + d5.length);
        } catch (IllegalArgumentException e) {
            System.out.println("T5: " + e.getMessage());
        }
    }
}