import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Task66 {

    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(Arrays.asList(
            "echo", "date", "uname", "whoami", "pwd", "ls"
    ));
    private static final Set<String> ALLOWED_UNAME_FLAGS = new HashSet<>(Arrays.asList("-a", "-s", "-r", "-m"));
    private static final Set<String> ALLOWED_LS_FLAGS = new HashSet<>(Arrays.asList("-l", "-a", "-la", "-al"));
    private static final Pattern SAFE_ECHO_TOKEN = Pattern.compile("^[A-Za-z0-9._,@:+-]{1,64}$");

    private static final int MAX_INPUT_LENGTH = 256;
    private static final int MAX_OUTPUT_BYTES = 100_000;
    private static final long TIMEOUT_SECONDS = 5;

    public static String executeValidatedCommand(String baseDir, String userInput) {
        if (baseDir == null || baseDir.isEmpty()) {
            return "ERROR: invalid base directory.";
        }
        if (userInput == null) {
            return "ERROR: null input.";
        }
        String input = userInput.trim();
        if (input.isEmpty() || input.length() > MAX_INPUT_LENGTH) {
            return "ERROR: input length invalid.";
        }
        if (hasForbiddenChars(input)) {
            return "ERROR: input contains forbidden characters.";
        }

        String[] tokens = input.split("\\s+");
        if (tokens.length == 0) {
            return "ERROR: empty command.";
        }

        String cmd = tokens[0];
        if (!ALLOWED_COMMANDS.contains(cmd)) {
            return "ERROR: command not allowed.";
        }

        List<String> command = new ArrayList<>();
        switch (cmd) {
            case "echo": {
                command.add("echo");
                for (int i = 1; i < tokens.length; i++) {
                    String t = tokens[i];
                    if (!SAFE_ECHO_TOKEN.matcher(t).matches()) {
                        return "ERROR: echo argument contains invalid characters or length.";
                    }
                    command.add(t);
                }
                break;
            }
            case "date": {
                if (tokens.length != 1) return "ERROR: 'date' takes no arguments.";
                command.add("date");
                break;
            }
            case "whoami": {
                if (tokens.length != 1) return "ERROR: 'whoami' takes no arguments.";
                command.add("whoami");
                break;
            }
            case "pwd": {
                if (tokens.length != 1) return "ERROR: 'pwd' takes no arguments.";
                command.add("pwd");
                break;
            }
            case "uname": {
                if (tokens.length == 1) {
                    command.add("uname");
                } else if (tokens.length == 2 && ALLOWED_UNAME_FLAGS.contains(tokens[1])) {
                    command.add("uname");
                    command.add(tokens[1]);
                } else {
                    return "ERROR: invalid 'uname' usage.";
                }
                break;
            }
            case "ls": {
                if (tokens.length == 1) {
                    command.add("ls");
                } else if (tokens.length == 2 && ALLOWED_LS_FLAGS.contains(tokens[1])) {
                    command.add("ls");
                    command.add(tokens[1]);
                } else {
                    return "ERROR: only simple 'ls' flags allowed (no paths).";
                }
                break;
            }
            default:
                return "ERROR: command not allowed.";
        }

        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            pb.redirectErrorStream(true);
            pb.directory(new File(baseDir));
            Process proc = pb.start();

            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            Thread reader = new Thread(() -> {
                try (InputStream in = new BufferedInputStream(proc.getInputStream())) {
                    byte[] buf = new byte[4096];
                    int read;
                    int total = 0;
                    while ((read = in.read(buf)) != -1) {
                        int toWrite = Math.min(read, MAX_OUTPUT_BYTES - total);
                        if (toWrite > 0) {
                            baos.write(buf, 0, toWrite);
                            total += toWrite;
                        }
                        if (total >= MAX_OUTPUT_BYTES) break;
                    }
                } catch (Exception ignored) { }
            });
            reader.setDaemon(true);
            reader.start();

            boolean finished = proc.waitFor(TIMEOUT_SECONDS, TimeUnit.SECONDS);
            if (!finished) {
                proc.destroyForcibly();
                reader.join(1000);
                return "ERROR: execution timed out.";
            }
            reader.join();
            int exit = proc.exitValue();
            String output = baos.toString(StandardCharsets.UTF_8);
            return "exit=" + exit + "\n" + output;
        } catch (Exception e) {
            return "ERROR: execution failure.";
        }
    }

    private static boolean hasForbiddenChars(String s) {
        // Disallow characters that enable shell features or redirection/pipelines
        String forbidden = ";|&`$()<>\"'\\";
        for (int i = 0; i < s.length(); i++) {
            if (forbidden.indexOf(s.charAt(i)) >= 0) return true;
        }
        return false;
    }

    public static void main(String[] args) {
        String baseDir = System.getProperty("user.dir");
        // 5 test cases
        String[] tests = {
                "echo Hello_World",
                "date",
                "uname -s",
                "ls -a",
                "ls /" // should be rejected (paths not allowed)
        };
        for (String t : tests) {
            String res = executeValidatedCommand(baseDir, t);
            System.out.println("TEST> " + t);
            System.out.println(res);
            System.out.println("----");
        }

        // Read a command from user input
        try {
            System.out.println("Enter a command:");
            Scanner sc = new Scanner(System.in, StandardCharsets.UTF_8.name());
            if (sc.hasNextLine()) {
                String line = sc.nextLine();
                String res = executeValidatedCommand(baseDir, line);
                System.out.println(res);
            }
        } catch (Exception e) {
            System.out.println("ERROR: failed to read input.");
        }
    }
}