import javax.naming.*;
import javax.naming.directory.*;
import java.util.*;

public class Task68 {

    public static String queryLdap(String dc, String username) {
        String baseDN = dcToBaseDN(dc);
        String filter = "(|(uid=" + ldapEscape(username) + ")(sAMAccountName=" + ldapEscape(username) + ")(cn=" + ldapEscape(username) + "))";

        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        env.put(Context.PROVIDER_URL, "ldap://localhost:389");
        DirContext ctx = null;
        try {
            ctx = new InitialDirContext(env);
            SearchControls sc = new SearchControls();
            sc.setSearchScope(SearchControls.SUBTREE_SCOPE);
            NamingEnumeration<SearchResult> results = ctx.search(baseDN, filter, sc);

            if (results.hasMore()) {
                SearchResult sr = results.next();
                StringBuilder out = new StringBuilder();
                out.append("{dn:\"").append(sr.getNameInNamespace()).append("\",attributes:{");
                Attributes attrs = sr.getAttributes();
                boolean firstAttr = true;
                NamingEnumeration<? extends Attribute> all = attrs.getAll();
                while (all.hasMore()) {
                    Attribute at = all.next();
                    if (!firstAttr) out.append(",");
                    firstAttr = false;
                    out.append("\"").append(at.getID()).append("\":[");
                    boolean firstVal = true;
                    NamingEnumeration<?> vals = at.getAll();
                    while (vals.hasMore()) {
                        Object val = vals.next();
                        if (!firstVal) out.append(",");
                        firstVal = false;
                        out.append("\"").append(String.valueOf(val).replace("\"", "\\\"")).append("\"");
                    }
                    out.append("]");
                }
                out.append("}}");
                return out.toString();
            } else {
                return "NOT_FOUND";
            }
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        } finally {
            if (ctx != null) {
                try { ctx.close(); } catch (Exception ignore) {}
            }
        }
    }

    private static String dcToBaseDN(String dc) {
        if (dc == null) return "";
        String trimmed = dc.trim();
        if (trimmed.toLowerCase().contains("dc=")) return trimmed;
        String[] parts = trimmed.split("[\\.,]");
        StringBuilder sb = new StringBuilder();
        for (String p : parts) {
            String part = p.trim();
            if (part.isEmpty()) continue;
            if (sb.length() > 0) sb.append(",");
            sb.append("dc=").append(part);
        }
        return sb.toString();
    }

    private static String ldapEscape(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder();
        for (char c : s.toCharArray()) {
            switch (c) {
                case '\\': sb.append("\\5c"); break;
                case '*': sb.append("\\2a"); break;
                case '(': sb.append("\\28"); break;
                case ')': sb.append("\\29"); break;
                case '\0': sb.append("\\00"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        if (args.length >= 2) {
            System.out.println(queryLdap(args[0], args[1]));
            return;
        }
        String[][] tests = new String[][]{
                {"example.com", "alice"},
                {"example.com", "bob"},
                {"dc=example,dc=com", "carol"},
                {"example,org", "dave"},
                {"test.local", "eve"}
        };
        for (String[] t : tests) {
            System.out.println("DC=" + t[0] + " USER=" + t[1]);
            System.out.println(queryLdap(t[0], t[1]));
        }
    }
}