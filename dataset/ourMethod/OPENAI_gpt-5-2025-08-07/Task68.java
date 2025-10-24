import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.*;
import java.util.Hashtable;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import java.nio.charset.StandardCharsets;

public class Task68 {

    public static String searchUser(String domain, String username) {
        String validatedDomain = validateDomain(domain);
        if (validatedDomain == null) {
            return "ERROR: Invalid domain format.";
        }
        String validatedUser = validateUsername(username);
        if (validatedUser == null) {
            return "ERROR: Invalid username format.";
        }

        String baseDN = buildBaseDN(validatedDomain);
        String filterUser = escapeLDAPFilter(validatedUser);
        String filter = "(&(objectClass=person)(|(uid=" + filterUser + ")(sAMAccountName=" + filterUser + ")(cn=" + filterUser + ")))";

        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        env.put(Context.PROVIDER_URL, "ldap://localhost:389/" + baseDN);
        env.put("java.naming.ldap.version", "3");
        env.put("com.sun.jndi.ldap.connect.timeout", "3000");
        env.put("com.sun.jndi.ldap.read.timeout", "5000");
        env.put(Context.SECURITY_AUTHENTICATION, "none"); // anonymous

        DirContext ctx = null;
        NamingEnumeration<SearchResult> results = null;
        try {
            ctx = new InitialDirContext(env);
            SearchControls ctrls = new SearchControls();
            ctrls.setSearchScope(SearchControls.SUBTREE_SCOPE);
            ctrls.setCountLimit(50);
            ctrls.setTimeLimit(4000);
            String[] attrs = new String[] {"dn", "cn", "uid", "sAMAccountName", "mail", "givenName", "sn"};
            ctrls.setReturningAttributes(attrs);

            results = ctx.search(baseDN, filter, ctrls);
            List<String> entries = new ArrayList<>();
            while (results.hasMore()) {
                SearchResult sr = results.next();
                String dn = sr.getNameInNamespace();
                Attributes a = sr.getAttributes();
                String cn = getAttr(a, "cn");
                String uid = getAttr(a, "uid");
                String sam = getAttr(a, "sAMAccountName");
                String mail = getAttr(a, "mail");
                String given = getAttr(a, "givenName");
                String sn = getAttr(a, "sn");
                String entry = "{\"dn\":\"" + safeJson(dn) + "\"," +
                        "\"cn\":\"" + safeJson(cn) + "\"," +
                        "\"uid\":\"" + safeJson(uid) + "\"," +
                        "\"sAMAccountName\":\"" + safeJson(sam) + "\"," +
                        "\"mail\":\"" + safeJson(mail) + "\"," +
                        "\"givenName\":\"" + safeJson(given) + "\"," +
                        "\"sn\":\"" + safeJson(sn) + "\"}";
                entries.add(entry);
            }
            if (entries.isEmpty()) {
                return "NOT_FOUND";
            }
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            for (int i = 0; i < entries.size(); i++) {
                if (i > 0) sb.append(",");
                sb.append(entries.get(i));
            }
            sb.append("]");
            return sb.toString();
        } catch (NamingException e) {
            return "ERROR: " + safeJson(e.getMessage() == null ? "LDAP operation failed." : e.getMessage());
        } finally {
            if (results != null) {
                try { results.close(); } catch (Exception ignored) {}
            }
            if (ctx != null) {
                try { ctx.close(); } catch (Exception ignored) {}
            }
        }
    }

    private static String validateDomain(String domain) {
        if (domain == null) return null;
        String d = domain.trim().toLowerCase();
        if (d.length() < 3 || d.length() > 253) return null;
        String[] labels = d.split("\\.");
        if (labels.length < 1) return null;
        Pattern p = Pattern.compile("^[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?$");
        for (String label : labels) {
            if (label.isEmpty() || label.length() > 63) return null;
            if (!p.matcher(label).matches()) return null;
        }
        return d;
    }

    private static String validateUsername(String username) {
        if (username == null) return null;
        String u = username.trim();
        if (u.isEmpty() || u.length() > 64) return null;
        if (!Pattern.compile("^[A-Za-z0-9._-]+$").matcher(u).matches()) return null;
        return u;
    }

    private static String buildBaseDN(String domain) {
        String[] parts = domain.split("\\.");
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < parts.length; i++) {
            if (i > 0) sb.append(",");
            sb.append("dc=").append(escapeLDAPDN(parts[i]));
        }
        return sb.toString();
    }

    private static String escapeLDAPFilter(String input) {
        // RFC 4515: escape \ * ( ) NUL
        StringBuilder sb = new StringBuilder(input.length());
        for (char c : input.toCharArray()) {
            switch (c) {
                case '\\': sb.append("\\5c"); break;
                case '*': sb.append("\\2a"); break;
                case '(': sb.append("\\28"); break;
                case ')': sb.append("\\29"); break;
                case '\u0000': sb.append("\\00"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    private static String escapeLDAPDN(String value) {
        // Minimal RFC 4514 escaping for DN attribute values
        String v = value;
        StringBuilder sb = new StringBuilder(v.length() + 8);
        for (int i = 0; i < v.length(); i++) {
            char c = v.charAt(i);
            switch (c) {
                case ',':
                case '+':
                case '"':
                case '\\':
                case '<':
                case '>':
                case ';':
                case '#':
                case '=':
                    sb.append('\\').append(c);
                    break;
                default:
                    sb.append(c);
            }
        }
        // Escape leading or trailing spaces
        if (sb.length() > 0) {
            if (sb.charAt(0) == ' ') sb.insert(0, '\\');
            if (sb.charAt(sb.length() - 1) == ' ') sb.insert(sb.length() - 1, '\\');
        }
        return sb.toString();
    }

    private static String getAttr(Attributes attrs, String name) {
        if (attrs == null) return "";
        Attribute a = attrs.get(name);
        if (a == null) return "";
        try {
            Object v = a.get();
            if (v == null) return "";
            String s = v.toString();
            byte[] b = s.getBytes(StandardCharsets.UTF_8);
            return new String(b, StandardCharsets.UTF_8);
        } catch (NamingException e) {
            return "";
        }
    }

    private static String safeJson(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '"': out.append("\\\""); break;
                case '\\': out.append("\\\\"); break;
                case '/': out.append("\\/"); break;
                case '\b': out.append("\\b"); break;
                case '\f': out.append("\\f"); break;
                case '\n': out.append("\\n"); break;
                case '\r': out.append("\\r"); break;
                case '\t': out.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        out.append(String.format("\\u%04x", (int)c));
                    } else {
                        out.append(c);
                    }
            }
        }
        return out.toString();
    }

    public static void main(String[] args) {
        if (args != null && args.length >= 2) {
            String dc = args[0];
            String user = args[1];
            System.out.println(searchUser(dc, user));
            return;
        }
        // Built-in test cases (will attempt LDAP on localhost; results depend on server)
        String[][] tests = new String[][]{
                {"example.com", "alice"},
                {"corp.local", "bob_smith"},
                {"test.org", "john.doe"},
                {"example.com", "invalid*user"}, // invalid username should be rejected
                {"bad_domain", "charlie"} // invalid domain should be rejected
        };
        for (String[] t : tests) {
            System.out.println("Input: dc=" + t[0] + " user=" + t[1]);
            System.out.println(searchUser(t[0], t[1]));
            System.out.println("---");
        }
    }
}