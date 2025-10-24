import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

import java.security.SecureRandom;
import java.time.Instant;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task49 {

    // Validation patterns
    private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z][A-Za-z\\s\\-']{0,98}[A-Za-z]$");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^(?=.{3,254}$)[A-Za-z0-9._%+\\-]+@[A-Za-z0-9\\-]+(?:\\.[A-Za-z0-9\\-]+)+$");
    private static final int MAX_NAME_LEN = 100;

    private static final SecureRandom RNG = new SecureRandom();
    private static final Path DB_PATH = Paths.get("users_db_java.jsonl");

    public static String createUser(String name, String email, int age) {
        // Validate inputs
        String n = safeTrim(name);
        String e = safeTrim(email);

        if (!isValidName(n)) {
            return "ERROR: invalid name";
        }
        if (!isValidEmail(e)) {
            return "ERROR: invalid email";
        }
        if (!isValidAge(age)) {
            return "ERROR: invalid age";
        }

        String id = generateId();
        String json = "{\"id\":\"" + id + "\",\"name\":\"" + escapeJson(n) + "\",\"email\":\"" + escapeJson(e) + "\",\"age\":" + age + ",\"ts\":\"" + Instant.now().toString() + "\"}\n";
        try {
            appendLineToDb(json);
            return "OK: " + id;
        } catch (IOException ex) {
            return "ERROR: storage failure";
        }
    }

    private static boolean isValidName(String name) {
        if (name == null) return false;
        name = name.strip();
        if (name.length() < 2 || name.length() > MAX_NAME_LEN) return false;
        Matcher m = NAME_PATTERN.matcher(name);
        return m.matches();
    }

    private static boolean isValidEmail(String email) {
        if (email == null) return false;
        email = email.strip().toLowerCase(Locale.ROOT);
        Matcher m = EMAIL_PATTERN.matcher(email);
        return m.matches();
    }

    private static boolean isValidAge(int age) {
        return age >= 0 && age <= 150;
    }

    private static String safeTrim(String s) {
        return s == null ? "" : s.trim();
    }

    private static String escapeJson(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '\\': sb.append("\\\\"); break;
                case '"': sb.append("\\\""); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int) c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    private static String generateId() {
        byte[] buf = new byte[8];
        RNG.nextBytes(buf);
        StringBuilder sb = new StringBuilder(16);
        for (byte b : buf) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    private static void appendLineToDb(String line) throws IOException {
        // Ensure file exists
        try {
            Files.createFile(DB_PATH);
        } catch (FileAlreadyExistsException ignore) {
        }
        try (FileChannel ch = FileChannel.open(DB_PATH, StandardOpenOption.WRITE, StandardOpenOption.APPEND)) {
            try (var lock = ch.lock()) {
                ByteBuffer bb = ByteBuffer.wrap(line.getBytes(StandardCharsets.UTF_8));
                while (bb.hasRemaining()) {
                    ch.write(bb);
                }
                ch.force(true);
            }
        }
    }

    // Five test cases
    public static void main(String[] args) {
        System.out.println(createUser("John Doe", "john.doe@example.com", 30));
        System.out.println(createUser("A", "bademail", 25)); // invalid name and email
        System.out.println(createUser("Mary-Jane O'Neil", "mary.oneil@example.org", -5)); // invalid age
        System.out.println(createUser("  Robert   ", "robert.smith@example.co.uk", 44));
        System.out.println(createUser("ThisNameIsWayTooLong_ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "long.name@example.com", 20));
    }
}