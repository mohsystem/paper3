import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;

public class Lse110 {

    // Validate username: length 1..32, ASCII only, starts with [A-Za-z_],
    // then [A-Za-z0-9_.-], optional trailing '$'
    public static boolean isValidUsername(String u) {
        if (u == null) return false;
        if (u.length() < 1 || u.length() > 32) return false;
        // Enforce ASCII only
        for (int i = 0; i < u.length(); i++) {
            if ((u.charAt(i) & 0x80) != 0) return false;
        }
        boolean hasTrailingDollar = u.endsWith("$");
        String base = hasTrailingDollar ? u.substring(0, u.length() - 1) : u;
        if (base.length() < 1) return false;

        char first = base.charAt(0);
        if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z') || first == '_')) {
            return false;
        }
        for (int i = 1; i < base.length(); i++) {
            char c = base.charAt(i);
            boolean ok =
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || c == '-' || c == '.';
            if (!ok) return false;
        }
        return true;
    }

    // Run `id <username>` securely and return exit status
    public static int runIdCommand(String username) {
        if (!isValidUsername(username)) {
            System.err.println("Invalid username.");
            return 2; // invalid input
        }
        ProcessBuilder pb = new ProcessBuilder(Arrays.asList("id", username));
        pb.redirectError(ProcessBuilder.Redirect.INHERIT);
        pb.redirectOutput(ProcessBuilder.Redirect.INHERIT);
        try {
            Process p = pb.start();
            int rc = p.waitFor();
            return rc;
        } catch (SecurityException se) {
            return 126; // permission problem
        } catch (Exception e) {
            return 127; // command not found or other execution error
        }
    }

    public static void main(String[] args) {
        if (args.length >= 1) {
            String username = new String(args[0].getBytes(StandardCharsets.UTF_8), StandardCharsets.UTF_8);
            int status = runIdCommand(username);
            System.out.println("Exit status: " + status);
            return;
        }
        // 5 test cases (may vary by system)
        List<String> tests = Arrays.asList("root", "daemon", "nobody", "bin", "this_user_should_not_exist_zzz");
        for (String t : tests) {
            System.out.println("Running: id " + t);
            int status = runIdCommand(t);
            System.out.println("Exit status: " + status);
        }
    }
}