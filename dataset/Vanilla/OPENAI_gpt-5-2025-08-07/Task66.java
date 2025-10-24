import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;

public class Task66 {
    private static final HashSet<String> ALLOWED = new HashSet<>(Arrays.asList(
            "echo", "ls", "pwd", "whoami", "date", "uname", "cat"
    ));
    private static final String PROHIBITED = "<>|;&$`(){}[]";

    public static String runCommand(String command) {
        try {
            List<String> tokens = tokenize(command);
            if (tokens.isEmpty()) {
                return "Invalid command: empty input";
            }
            String validationError = validateTokens(tokens);
            if (validationError != null) {
                return "Invalid command: " + validationError;
            }
            ProcessBuilder pb = new ProcessBuilder(tokens);
            pb.redirectErrorStream(true);
            Process p = pb.start();
            String out = readAll(p.getInputStream());
            int code = p.waitFor();
            return out;
        } catch (IllegalArgumentException e) {
            return "Invalid command: " + e.getMessage();
        } catch (Exception e) {
            return "Execution error: " + e.getMessage();
        }
    }

    private static String readAll(InputStream is) throws Exception {
        BufferedReader br = new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8));
        StringBuilder sb = new StringBuilder();
        char[] buf = new char[4096];
        int n;
        while ((n = br.read(buf)) != -1) {
            sb.append(buf, 0, n);
        }
        return sb.toString();
    }

    private static List<String> tokenize(String s) {
        ArrayList<String> tokens = new ArrayList<>();
        StringBuilder cur = new StringBuilder();
        boolean inSingle = false, inDouble = false, esc = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (esc) {
                cur.append(c);
                esc = false;
            } else if (inSingle) {
                if (c == '\'') inSingle = false;
                else cur.append(c);
            } else if (inDouble) {
                if (c == '"') inDouble = false;
                else if (c == '\\') esc = true;
                else cur.append(c);
            } else {
                if (Character.isWhitespace(c)) {
                    if (cur.length() > 0) {
                        tokens.add(cur.toString());
                        cur.setLength(0);
                    }
                } else if (c == '\\') {
                    esc = true;
                } else if (c == '\'') {
                    inSingle = true;
                } else if (c == '"') {
                    inDouble = true;
                } else {
                    cur.append(c);
                }
            }
        }
        if (esc) throw new IllegalArgumentException("unfinished escape");
        if (inSingle || inDouble) throw new IllegalArgumentException("unmatched quotes");
        if (cur.length() > 0) tokens.add(cur.toString());
        return tokens;
    }

    private static String validateTokens(List<String> tokens) {
        if (tokens.size() == 0) return "empty";
        if (tokens.size() > 32) return "too many arguments";
        String cmd = tokens.get(0);
        if (!ALLOWED.contains(cmd)) return "command not allowed";
        for (String t : tokens) {
            for (int i = 0; i < t.length(); i++) {
                if (PROHIBITED.indexOf(t.charAt(i)) >= 0) {
                    return "contains prohibited characters";
                }
            }
        }
        if (cmd.equals("cat") || cmd.equals("ls")) {
            for (int i = 1; i < tokens.size(); i++) {
                String a = tokens.get(i);
                if (a.startsWith("-")) continue;
                if (a.startsWith("/")) return "absolute paths are not allowed";
                if (a.contains("..")) return "path traversal is not allowed";
                if (a.contains("/")) return "subdirectories not allowed in arguments";
            }
        }
        return null;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "echo Hello World",
                "pwd",
                "date",
                "uname -s",
                "ls -1"
        };
        for (String t : tests) {
            System.out.println("Command: " + t);
            String out = runCommand(t);
            System.out.println(out);
            System.out.println("-----");
        }
    }
}