import java.util.*;

public class Lse131 {
    private static String escapeHtml(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(Math.min(1024, s.length() * 2));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    public static String render_response_from_env(Map<String, String> env, Map<String, String> request) {
        if (env == null) env = Collections.emptyMap();
        if (request == null) request = Collections.emptyMap();
        String name = request.getOrDefault("name", "");
        String template = env.get("template");
        if (template == null || template.length() > 10000) {
            template = "Hello, {{name}}";
        }
        String escapedName = escapeHtml(name);
        String token = "{{name}}";
        StringBuilder res = new StringBuilder(Math.min(16384, template.length() + Math.max(0, escapedName.length() - token.length())));
        int idx = 0;
        while (true) {
            int pos = template.indexOf(token, idx);
            if (pos < 0) {
                res.append(template.substring(idx));
                break;
            }
            res.append(template, idx, pos);
            res.append(escapedName);
            idx = pos + token.length();
        }
        return res.toString();
    }

    public static String home(Map<String, String> env, Map<String, String> request) {
        return render_response_from_env(env, request);
    }

    public static void main(String[] args) {
        Map<String, String> env = new HashMap<>();
        env.put("template", "<h1>Welcome, {{name}}</h1>");

        List<Map<String, String>> tests = new ArrayList<>();
        Map<String, String> r1 = new HashMap<>(); r1.put("name", "Alice"); tests.add(r1);
        Map<String, String> r2 = new HashMap<>(); r2.put("name", "<script>alert(1)</script>"); tests.add(r2);
        Map<String, String> r3 = new HashMap<>(); tests.add(r3);
        Map<String, String> r4 = new HashMap<>(); r4.put("name", "O'Reilly & Co"); tests.add(r4);
        Map<String, String> r5 = new HashMap<>(); r5.put("name", "A".repeat(50)); tests.add(r5);

        for (int i = 0; i < tests.size(); i++) {
            String resp = home(env, tests.get(i));
            System.out.println("Test " + (i + 1) + ": " + resp);
        }
    }
}