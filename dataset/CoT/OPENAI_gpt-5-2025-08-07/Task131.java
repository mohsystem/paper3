// Chain-of-Through process in code generation:
// 1) Problem understanding: Safely deserialize user-supplied data in a constrained custom format.
//    Expected input format: "name=<text>;age=<int>;email=<text>;roles=<comma,separated,roles>"
//    Output: Canonical JSON string or an error message. Must validate and constrain input to avoid insecure deserialization.
// 2) Security requirements: 
//    - No native object deserialization of untrusted data.
//    - Strict length limits, strict schema, type checks, allowlist for roles, character whitelisting, duplicate/unknown key rejection.
// 3) Secure coding generation: Implement defensive parsing and validation.
// 4) Code review: Avoid regex DoS; use simple scanners. Enforce bounds and handle errors clearly.
// 5) Secure code output: Final code below applies validations and returns canonical JSON or error.
//
// Java implementation
import java.util.*;

public class Task131 {
    private static final int MAX_INPUT_LENGTH = 1024;
    private static final Set<String> ALLOWED_KEYS = new HashSet<>(Arrays.asList("name", "age", "email", "roles"));
    private static final Set<String> ALLOWED_ROLES = new HashSet<>(Arrays.asList("user","admin","moderator","viewer","editor"));

    public static String deserializeUserData(String input) {
        if (input == null) return "ERROR: null input";
        if (input.length() == 0) return "ERROR: empty input";
        if (input.length() > MAX_INPUT_LENGTH) return "ERROR: input too long";
        Map<String, String> map = new HashMap<>();
        String[] pairs = input.split(";", -1);
        if (pairs.length > 16) return "ERROR: too many fields";
        for (String pair : pairs) {
            if (pair.isEmpty()) continue;
            int eq = pair.indexOf('=');
            if (eq <= 0 || eq == pair.length()-1) return "ERROR: invalid key=value pair";
            String key = pair.substring(0, eq).trim();
            String val = pair.substring(eq+1).trim();
            if (!ALLOWED_KEYS.contains(key)) return "ERROR: unknown key: " + key;
            if (map.containsKey(key)) return "ERROR: duplicate key: " + key;
            if (val.isEmpty()) return "ERROR: empty value for " + key;
            map.put(key, val);
        }
        for (String req : ALLOWED_KEYS) {
            if (!map.containsKey(req)) return "ERROR: missing required field: " + req;
        }
        String name = map.get("name");
        String ageStr = map.get("age");
        String email = map.get("email");
        String rolesStr = map.get("roles");

        if (!isValidName(name)) return "ERROR: invalid name";
        Integer age = parseAge(ageStr);
        if (age == null) return "ERROR: invalid age";
        if (!isValidEmail(email)) return "ERROR: invalid email";
        List<String> roles = parseRoles(rolesStr);
        if (roles == null) return "ERROR: invalid roles";

        // Build canonical JSON-like string safely
        StringBuilder sb = new StringBuilder();
        sb.append("{\"name\":\"").append(escapeJson(name)).append("\",");
        sb.append("\"age\":").append(age).append(",");
        sb.append("\"email\":\"").append(escapeJson(email)).append("\",");
        sb.append("\"roles\":[");
        for (int i = 0; i < roles.size(); i++) {
            if (i > 0) sb.append(",");
            sb.append("\"").append(roles.get(i)).append("\"");
        }
        sb.append("]}");
        return sb.toString();
    }

    private static boolean isValidName(String s) {
        if (s.length() < 1 || s.length() > 50) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!(Character.isLetter(c) || c==' ' || c=='-' || c=='\'')) return false;
        }
        return true;
    }

    private static Integer parseAge(String s) {
        if (s.length() < 1 || s.length() > 3) return null;
        int val = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!Character.isDigit(c)) return null;
            val = val*10 + (c - '0');
            if (val > 150) return null;
        }
        if (val < 0 || val > 120) return null;
        return val;
    }

    private static boolean isValidEmail(String s) {
        if (s.length() < 6 || s.length() > 100) return false;
        int at = s.indexOf('@');
        if (at <= 0 || at != s.lastIndexOf('@') || at >= s.length()-1) return false;
        String local = s.substring(0, at);
        String domain = s.substring(at+1);
        if (!checkLocal(local)) return false;
        if (!checkDomain(domain)) return false;
        return true;
    }

    private static boolean checkLocal(String s) {
        if (s.length() == 0) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='.'||c=='_'||c=='%'||c=='+'||c=='-';
            if (!ok) return false;
        }
        return true;
    }

    private static boolean checkDomain(String s) {
        if (s.length() < 3 || s.length() > 100) return false;
        if (s.startsWith(".") || s.endsWith(".")) return false;
        if (s.indexOf("..") >= 0) return false;
        int lastDot = s.lastIndexOf('.');
        if (lastDot <= 0 || lastDot == s.length()-1) return false;
        String tld = s.substring(lastDot+1);
        if (tld.length() < 2 || tld.length() > 10) return false;
        for (int i=0;i<s.length();i++){
            char c = s.charAt(i);
            boolean ok = (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='.'||c=='-';
            if (!ok) return false;
        }
        for (int i=0;i<tld.length();i++){
            char c = tld.charAt(i);
            if (!Character.isLetter(c)) return false;
        }
        return true;
    }

    private static List<String> parseRoles(String s) {
        if (s.length() == 0 || s.length() > 200) return null;
        String[] parts = s.split(",", -1);
        if (parts.length == 0 || parts.length > 5) return null;
        LinkedHashSet<String> out = new LinkedHashSet<>();
        for (String r : parts) {
            String role = r.trim();
            if (role.isEmpty()) return null;
            if (!ALLOWED_ROLES.contains(role)) return null;
            out.add(role);
        }
        return new ArrayList<>(out);
    }

    private static String escapeJson(String s) {
        StringBuilder sb = new StringBuilder(s.length()+8);
        for (int i=0;i<s.length();i++){
            char c = s.charAt(i);
            switch(c){
                case '"': sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x",(int)c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
            "name=Alice Johnson;age=30;email=alice.j@example.com;roles=user,editor",
            "name=Bob;age=45;email=bob@example.org;roles=admin,moderator,viewer",
            "name=Carol;age=abc;email=carol@example.com;roles=user", // invalid age
            "name=Dan;age=25;email=dan@bad_domain;roles=user", // invalid email
            "name=Eve;age=29;email=eve@example.com;roles=user;token=xyz" // unknown key
        };
        for (String t : tests) {
            System.out.println(deserializeUserData(t));
        }
        // Oversized input case
        char[] big = new char[1100];
        Arrays.fill(big, 'a');
        String bigStr = "name=" + new String(big) + ";age=20;email=aa@bb.cc;roles=user";
        System.out.println(deserializeUserData(bigStr));
    }
}