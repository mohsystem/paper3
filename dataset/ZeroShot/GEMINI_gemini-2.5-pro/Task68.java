import javax.naming.*;
import javax.naming.directory.*;
import java.util.Hashtable;

public class Task68 {

    /**
     * Escapes characters in an LDAP search filter that are special according to RFC 4515.
     * Special characters are: '(', ')', '\', '*', NUL.
     *
     * @param filter The string to escape.
     * @return The escaped string.
     */
    public static String escapeLDAPSearchFilter(String filter) {
        if (filter == null) {
            return null;
        }
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < filter.length(); i++) {
            char curChar = filter.charAt(i);
            switch (curChar) {
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
                    sb.append(curChar);
            }
        }
        return sb.toString();
    }

    /**
     * Searches for a user in an LDAP directory.
     *
     * @param dcString A comma-separated string of domain components (e.g., "example,com").
     * @param username The username to search for.
     * @return A string containing user information or an error/not found message.
     */
    public static String searchLdapUser(String dcString, String username) {
        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        env.put(Context.PROVIDER_URL, "ldap://localhost:389");
        // For simplicity, using anonymous bind. In a real application, use authentication.
        env.put(Context.SECURITY_AUTHENTICATION, "none");

        DirContext ctx = null;
        NamingEnumeration<?> results = null;
        StringBuilder resultBuilder = new StringBuilder();

        try {
            ctx = new InitialDirContext(env);

            String searchBase = "dc=" + dcString.replace(",", ",dc=");
            String escapedUsername = escapeLDAPSearchFilter(username);
            String searchFilter = "(cn=" + escapedUsername + ")";

            SearchControls searchControls = new SearchControls();
            searchControls.setSearchScope(SearchControls.SUBTREE_SCOPE);
            
            resultBuilder.append("Searching for user '").append(username).append("'\n");
            resultBuilder.append("Base DN: ").append(searchBase).append("\n");
            resultBuilder.append("Filter: ").append(searchFilter).append("\n\n");

            results = ctx.search(searchBase, searchFilter, searchControls);

            if (results.hasMore()) {
                SearchResult searchResult = (SearchResult) results.next();
                resultBuilder.append("Found user: ").append(searchResult.getNameInNamespace()).append("\n");
                Attributes attributes = searchResult.getAttributes();
                NamingEnumeration<? extends Attribute> allAttributes = attributes.getAll();
                while (allAttributes.hasMore()) {
                    Attribute attr = allAttributes.next();
                    resultBuilder.append(attr.getID()).append(": ").append(attr.get()).append("\n");
                }
            } else {
                resultBuilder.append("User '").append(username).append("' not found.\n");
            }

        } catch (AuthenticationException e) {
            return "LDAP Authentication failed: " + e.toString();
        } catch (NamingException e) {
            return "LDAP search failed: " + e.toString();
        } finally {
            if (results != null) {
                try {
                    results.close();
                } catch (Exception e) { /* ignore */ }
            }
            if (ctx != null) {
                try {
                    ctx.close();
                } catch (Exception e) { /* ignore */ }
            }
        }
        return resultBuilder.toString();
    }

    public static void main(String[] args) {
        System.out.println("NOTE: This program requires an LDAP server running on localhost:389.");
        System.out.println("For tests to succeed, it should have a base DN like 'dc=example,dc=com'");
        System.out.println("and users with cn='jdoe', 'test(user)', 'star*user', etc.\n");
        
        String[][] testCases = {
            {"jdoe", "example,com"},        // 1. Normal valid user
            {"nonexistent", "example,com"}, // 2. User not found
            {"test(user)", "example,com"},  // 3. Username with parentheses
            {"star*user", "example,com"},   // 4. Username with asterisk
            {"*", "example,com"}            // 5. Potential injection attempt
        };

        for (int i = 0; i < testCases.length; i++) {
            String username = testCases[i][0];
            String dcString = testCases[i][1];
            System.out.println("----------- Test Case " + (i + 1) + " -----------");
            String result = searchLdapUser(dcString, username);
            System.out.println(result);
        }
    }
}