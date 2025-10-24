import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.directory.*;
import javax.naming.ldap.InitialLdapContext;
import javax.naming.ldap.LdapContext;
import java.util.Hashtable;
import java.util.ArrayList;
import java.util.List;

public class Task68 {

    private static String escapeLDAPFilter(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder(input.length());
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '\\': sb.append("\\5c"); break;
                case '*':  sb.append("\\2a"); break;
                case '(':  sb.append("\\28"); break;
                case ')':  sb.append("\\29"); break;
                case '\u0000': sb.append("\\00"); break;
                default:
                    if (c < 0x20 || c == 0x7F) {
                        sb.append(String.format("\\%02x", (int) c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    private static String escapeDNValue(String value) {
        if (value == null) return "";
        // RFC 4514 minimal escaping
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < value.length(); i++) {
            char c = value.charAt(i);
            boolean mustEscape = false;
            if (i == 0 && (c == ' ' || c == '#')) mustEscape = true;
            if (i == value.length() - 1 && c == ' ') mustEscape = true;
            switch (c) {
                case ',': case '+': case '"': case '\\':
                case '<': case '>': case ';': case '=':
                    mustEscape = true; break;
                default:
                    // keep
            }
            if (mustEscape) {
                sb.append('\\').append(c);
            } else {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    private static boolean isPlausibleDomain(String domain) {
        if (domain == null) return false;
        if (domain.length() < 1 || domain.length() > 253) return false;
        if (domain.contains("..")) return false;
        String[] labels = domain.split("\\.");
        if (labels.length == 0) return false;
        for (String label : labels) {
            if (label.length() < 1 || label.length() > 63) return false;
            for (int i = 0; i < label.length(); i++) {
                char ch = label.charAt(i);
                if (!(Character.isLetterOrDigit(ch) || ch == '-')) return false;
            }
            if (label.startsWith("-") || label.endsWith("-")) return false;
        }
        return true;
    }

    private static boolean isPlausibleUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > 128) return false;
        for (int i = 0; i < username.length(); i++) {
            char ch = username.charAt(i);
            if (!(Character.isLetterOrDigit(ch) || ch == '.' || ch == '_' || ch == '-' )) {
                return false;
            }
        }
        return true;
    }

    private static String buildBaseDNFromDomain(String domain) {
        String[] parts = domain.split("\\.");
        List<String> rdns = new ArrayList<>();
        for (String p : parts) {
            rdns.add("dc=" + escapeDNValue(p));
        }
        return String.join(",", rdns);
    }

    public static String findUser(String domain, String username) {
        try {
            if (!isPlausibleDomain(domain)) {
                return "{\"error\":\"invalid_domain\"}";
            }
            if (!isPlausibleUsername(username)) {
                return "{\"error\":\"invalid_username\"}";
            }

            String baseDN = buildBaseDNFromDomain(domain);
            String filter = "(uid=" + escapeLDAPFilter(username) + ")";

            Hashtable<String, String> env = new Hashtable<>();
            env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
            env.put(Context.PROVIDER_URL, "ldap://127.0.0.1:389");
            env.put("com.sun.jndi.ldap.read.timeout", "5000");
            env.put("com.sun.jndi.ldap.connect.timeout", "5000");
            // Anonymous bind by default; if needed, set SECURITY_PRINCIPAL/SECURITY_CREDENTIALS.

            LdapContext ctx = new InitialLdapContext(env, null);

            SearchControls sc = new SearchControls();
            sc.setSearchScope(SearchControls.SUBTREE_SCOPE);
            sc.setCountLimit(1);
            sc.setReturningAttributes(new String[] { "cn", "uid", "mail" });

            NamingEnumeration<SearchResult> results = ctx.search(baseDN, filter, sc);
            if (!results.hasMore()) {
                try { ctx.close(); } catch (Exception ignore) {}
                return "{\"result\":null}";
            }
            SearchResult sr = results.next();
            Attributes attrs = sr.getAttributes();

            String dn = sr.getNameInNamespace();
            String cn = attrs.get("cn") != null ? attrs.get("cn").get().toString() : "";
            String uid = attrs.get("uid") != null ? attrs.get("uid").get().toString() : "";
            String mail = attrs.get("mail") != null ? attrs.get("mail").get().toString() : "";

            try { ctx.close(); } catch (Exception ignore) {}

            StringBuilder json = new StringBuilder();
            json.append("{");
            json.append("\"dn\":").append("\"").append(dn.replace("\\", "\\\\").replace("\"", "\\\"")).append("\",");
            json.append("\"cn\":").append("\"").append(cn.replace("\\", "\\\\").replace("\"", "\\\"")).append("\",");
            json.append("\"uid\":").append("\"").append(uid.replace("\\", "\\\\").replace("\"", "\\\"")).append("\",");
            json.append("\"mail\":").append("\"").append(mail.replace("\\", "\\\\").replace("\"", "\\\"")).append("\"");
            json.append("}");
            return json.toString();
        } catch (Exception e) {
            return "{\"error\":\"" + e.getClass().getSimpleName().replace("\"", "") + "\"}";
        }
    }

    public static void main(String[] args) {
        if (args != null && args.length >= 2) {
            System.out.println(findUser(args[0], args[1]));
            return;
        }
        // 5 test cases
        System.out.println(findUser("example.com", "jdoe"));
        System.out.println(findUser("example.com", "alice"));
        System.out.println(findUser("sub.example.com", "bob_smith"));
        System.out.println(findUser("invalid_domain!", "user"));
        System.out.println(findUser("example.com", "bad*user"));
    }
}