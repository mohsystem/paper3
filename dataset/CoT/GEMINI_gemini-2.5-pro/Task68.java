import javax.naming.*;
import javax.naming.directory.*;
import java.util.Hashtable;
import java.util.Enumeration;

public class Task68 {

    /**
     * Searches for a user in an LDAP directory. This function demonstrates secure
     * practices by using parameterized queries to prevent LDAP injection.
     *
     * NOTE: This code requires a running LDAP server (e.g., OpenLDAP) on localhost:389.
     * For the code to find a user, the server must be populated with data.
     * Example LDIF for a user:
     * dn: uid=jdoe,ou=people,dc=example,dc=com
     * objectClass: inetOrgPerson
     * cn: John Doe
     * sn: Doe
     * uid: jdoe
     * mail: jdoe@example.com
     *
     * @param username The username (uid) to search for.
     * @param domainComponent The domain component string, e.g., "example,com".
     * @return A string containing the search results, or an error message.
     */
    public static String searchLdapUser(String username, String domainComponent) {
        // --- 1. Set up connection environment ---
        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        env.put(Context.PROVIDER_URL, "ldap://localhost:389");
        // For production, use LDAPS ("ldaps://...:636") and configure SSL/TLS.
        // For authenticated access, provide principal and credentials:
        // env.put(Context.SECURITY_AUTHENTICATION, "simple");
        // env.put(Context.SECURITY_PRINCIPAL, "cn=admin,dc=example,dc=com");
        // env.put(Context.SECURITY_CREDENTIALS, "admin_password");

        DirContext ctx = null;
        StringBuilder resultBuilder = new StringBuilder();

        try {
            // --- 2. Create the initial directory context ---
            ctx = new InitialDirContext(env);

            // --- 3. Construct the search base from domain components ---
            // Basic validation to ensure the DC string contains only expected characters.
            if (domainComponent == null || !domainComponent.matches("^[a-zA-Z0-9,-.]+$")) {
                return "Error: Invalid domain component format.";
            }
            String searchBase = "dc=" + domainComponent.replace(",", ",dc=");

            // --- 4. Set up search controls ---
            SearchControls searchCtls = new SearchControls();
            searchCtls.setSearchScope(SearchControls.SUBTREE_SCOPE);
            // Specify which attributes to return to avoid exposing sensitive or unnecessary data.
            String[] returnedAtts = {"cn", "sn", "mail", "uid"};
            searchCtls.setReturningAttributes(returnedAtts);

            // --- 5. Construct the search filter using parameterization ---
            // This is the most secure way to build filters from user input as it prevents LDAP Injection.
            // The `{0}` is a placeholder for the first argument in the `Object[]` array.
            String searchFilter = "(uid={0})";

            // --- 6. Perform the search ---
            NamingEnumeration<SearchResult> answer = ctx.search(searchBase, searchFilter, new Object[]{username}, searchCtls);

            if (!answer.hasMoreElements()) {
                resultBuilder.append("User '").append(username).append("' not found in '").append(searchBase).append("'.\n");
            } else {
                resultBuilder.append("Search results for user '").append(username).append("':\n");
                while (answer.hasMoreElements()) {
                    SearchResult sr = answer.next();
                    resultBuilder.append(">> DN: ").append(sr.getNameInNamespace()).append("\n");
                    Attributes attrs = sr.getAttributes();
                    NamingEnumeration<? extends Attribute> allAttrs = attrs.getAll();
                    while (allAttrs.hasMore()) {
                        Attribute attr = allAttrs.next();
                        resultBuilder.append("   ").append(attr.getID()).append(": ");
                        Enumeration<?> values = attr.getAll();
                        while(values.hasMoreElements()){
                            resultBuilder.append(values.nextElement().toString()).append(" ");
                        }
                        resultBuilder.append("\n");
                    }
                }
            }
        } catch (NamingException e) {
            // In a production environment, log this exception with more detail.
            return "Error during LDAP search: " + e.getMessage();
        } finally {
            // --- 7. Close the context to release resources ---
            if (ctx != null) {
                try {
                    ctx.close();
                } catch (NamingException e) {
                    // Ignore or log this secondary exception.
                }
            }
        }
        return resultBuilder.toString();
    }

    public static void main(String[] args) {
        if (args.length == 2) {
            System.out.println("--- Running with Command Line Arguments ---");
            String username = args[0];
            String dc = args[1];
            System.out.println(searchLdapUser(username, dc));
        } else {
            System.out.println("Usage: java Task68 <username> <domain_component_string>");
            System.out.println("Example: java Task68 jdoe example,com\n");
            System.out.println("NOTE: This program requires a running LDAP server on localhost:389 with appropriate data.\n");

            // --- 5 Hardcoded Test Cases ---
            System.out.println("--- Running 5 Hardcoded Test Cases ---");

            // Test Case 1: A valid user that should exist on the test server.
            System.out.println("--- Test Case 1: Valid User ---");
            System.out.println(searchLdapUser("jdoe", "example,com"));

            // Test Case 2: A user that should not exist.
            System.out.println("--- Test Case 2: Non-existent User ---");
            System.out.println(searchLdapUser("nonexistentuser", "example,com"));

            // Test Case 3: A user in a different domain component.
            System.out.println("--- Test Case 3: User in a different DC ---");
            System.out.println(searchLdapUser("asmith", "test,org"));
            
            // Test Case 4: An attempt at LDAP injection. The parameterized query should handle this safely.
            System.out.println("--- Test Case 4: Potential Injection Attempt ---");
            System.out.println(searchLdapUser("jdoe)(uid=*", "example,com"));

            // Test Case 5: Invalid domain component format.
            System.out.println("--- Test Case 5: Invalid Domain Component ---");
            System.out.println(searchLdapUser("jdoe", "ex@mple,com"));
        }
    }
}