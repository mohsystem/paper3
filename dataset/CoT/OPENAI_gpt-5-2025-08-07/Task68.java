import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.directory.*;
import java.util.Hashtable;
import java.util.ArrayList;
import java.util.List;

public class Task68 {
    private static String escapeLDAPFilterValue(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder(input.length() * 2);
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '\\':
                    sb.append("\\5c");
                    break;
                case '*':
                    sb.append("\\2a");
                    break;
                case '(':
                    sb.append("\\28");
                    break;
                case ')':
                    sb.append("\\29");
                    break;
                case '\0':
                    sb.append("\\00");
                    break;
                default:
                    sb.append(c);
            }
        }
        return sb.toString();
    }

    private static boolean isValidLabel(String s) {
        if (s == null || s.isEmpty()) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         (c == '-') ;
            if (!ok) return false;
        }
        if (s.charAt(0) == '-' || s.charAt(s.length() - 1) == '-') return false;
        return true;
    }

    private static String buildBaseDN(String dcInput) {
        if (dcInput == null) return "dc=localhost";
        String in = dcInput.trim();
        if (in.isEmpty()) return "dc=localhost";

        if (in.contains("=")) {
            // Expect comma-separated dc= labels
            String[] parts = in.split("\\s*,\\s*");
            List<String> out = new ArrayList<>();
            for (String p : parts) {
                String[] kv = p.split("\\s*=\\s*", 2);
                if (kv.length != 2) continue;
                String k = kv[0].toLowerCase();
                String v = kv[1];
                if (!k.equals("dc")) continue;
                if (!isValidLabel(v)) continue;
                out.add("dc=" + v.toLowerCase());
            }
            if (out.isEmpty()) return "dc=localhost";
            return String.join(",", out);
        } else {
            String[] labels = in.split("\\.");
            List<String> out = new ArrayList<>();
            for (String label : labels) {
                String v = label.trim();
                if (v.isEmpty()) continue;
                if (!isValidLabel(v)) continue;
                out.add("dc=" + v.toLowerCase());
            }
            if (out.isEmpty()) return "dc=localhost";
            return String.join(",", out);
        }
    }

    public static String searchUser(String dcInput, String username) {
        String baseDn = buildBaseDN(dcInput);
        String userEsc = escapeLDAPFilterValue(username == null ? "" : username.trim());
        String filter = "(|(uid=" + userEsc + ")(cn=" + userEsc + ")(sAMAccountName=" + userEsc + ")(userPrincipalName=" + userEsc + "))";

        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        env.put(Context.PROVIDER_URL, "ldap://localhost:389");
        env.put(Context.REFERRAL, "ignore");
        env.put("com.sun.jndi.ldap.connect.timeout", "3000");
        env.put("com.sun.jndi.ldap.read.timeout", "5000");

        DirContext ctx = null;
        NamingEnumeration<SearchResult> results = null;
        try {
            ctx = new InitialDirContext(env);
            SearchControls controls = new SearchControls();
            controls.setSearchScope(SearchControls.SUBTREE_SCOPE);
            controls.setReturningAttributes(new String[] {
                    "cn", "sn", "givenName", "uid", "mail", "sAMAccountName", "userPrincipalName"
            });
            controls.setCountLimit(1);
            results = ctx.search(baseDn, filter, controls);
            if (results.hasMore()) {
                SearchResult sr = results.next();
                String dn = sr.getNameInNamespace();
                Attributes attrs = sr.getAttributes();
                StringBuilder json = new StringBuilder();
                json.append("{\"dn\":\"").append(dn.replace("\\", "\\\\").replace("\"", "\\\"")).append("\",\"attributes\":{");
                boolean first = true;
                NamingEnumeration<String> ids = attrs.getIDs();
                while (ids.hasMore()) {
                    String id = ids.next();
                    Attribute a = attrs.get(id);
                    if (a == null) continue;
                    if (!first) json.append(",");
                    first = false;
                    json.append("\"").append(id.replace("\\", "\\\\").replace("\"", "\\\"")).append("\":");
                    if (a.size() == 1) {
                        Object v = a.get();
                        String vs = v == null ? "" : v.toString();
                        json.append("\"").append(vs.replace("\\", "\\\\").replace("\"", "\\\"")).append("\"");
                    } else {
                        json.append("[");
                        for (int i = 0; i < a.size(); i++) {
                            Object v = a.get(i);
                            String vs = v == null ? "" : v.toString();
                            if (i > 0) json.append(",");
                            json.append("\"").append(vs.replace("\\", "\\\\").replace("\"", "\\\"")).append("\"");
                        }
                        json.append("]");
                    }
                }
                json.append("}}");
                return json.toString();
            } else {
                return "{\"error\":\"NOT_FOUND\"}";
            }
        } catch (Exception e) {
            String msg = e.getClass().getSimpleName() + ": " + (e.getMessage() == null ? "" : e.getMessage());
            return "{\"error\":\"" + msg.replace("\\", "\\\\").replace("\"", "\\\"") + "\"}";
        } finally {
            try {
                if (results != null) results.close();
            } catch (Exception ignored) {}
            try {
                if (ctx != null) ctx.close();
            } catch (Exception ignored) {}
        }
    }

    public static void main(String[] args) {
        if (args != null && args.length >= 2) {
            System.out.println(searchUser(args[0], args[1]));
            return;
        }
        String[][] tests = new String[][]{
                {"example.com", "alice"},
                {"dc=example,dc=com", "bob"},
                {"EXAMPLE.ORG", "charlie"},
                {"invalid$$domain", "dave"},
                {"dc=local,dc=domain", "eve"}
        };
        for (String[] t : tests) {
            System.out.println("Input: dc=" + t[0] + ", user=" + t[1]);
            System.out.println(searchUser(t[0], t[1]));
        }
    }
}