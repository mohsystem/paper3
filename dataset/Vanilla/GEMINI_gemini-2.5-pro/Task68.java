import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;
import javax.naming.ldap.InitialLdapContext;
import javax.naming.ldap.LdapContext;
import java.util.Hashtable;

public class Task68 {

    /**
     * Searches an LDAP directory for a user and returns their information.
     *
     * @param baseDn The base DN for the search (e.g., "dc=example,dc=com").
     * @param username The username (uid) to search for.
     * @return A string containing the user's information, or an error message.
     */
    public static String searchLdapUser(String baseDn, String username) {
        StringBuilder resultBuilder = new StringBuilder();
        LdapContext ctx = null;

        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        env.put(Context.PROVIDER_URL, "ldap://localhost:389"); // Assumes LDAP server on localhost

        try {
            ctx = new InitialLdapContext(env, null);

            SearchControls searchControls = new SearchControls();
            searchControls.setSearchScope(SearchControls.SUBTREE_SCOPE);

            String searchFilter = "(&(objectClass=person)(uid=" + username + "))";

            NamingEnumeration<SearchResult> answer = ctx.search(baseDn, searchFilter, searchControls);

            if (!answer.hasMoreElements()) {
                return "User '" + username + "' not found in '" + baseDn + "'.\n";
            }

            while (answer.hasMoreElements()) {
                SearchResult sr = answer.next();
                resultBuilder.append("Found entry: ").append(sr.getNameInNamespace()).append("\n");

                Attributes attrs = sr.getAttributes();
                NamingEnumeration<? extends Attribute> allAttrs = attrs.getAll();
                while (allAttrs.hasMore()) {
                    Attribute attr = allAttrs.next();
                    resultBuilder.append("  Attribute: ").append(attr.getID()).append("\n");
                    NamingEnumeration<?> values = attr.getAll();
                    while (values.hasMore()) {
                        resultBuilder.append("    Value: ").append(values.next()).append("\n");
                    }
                }
            }
        } catch (NamingException e) {
            return "LDAP Error: " + e.getMessage() + "\n";
        } finally {
            if (ctx != null) {
                try {
                    ctx.close();
                } catch (NamingException e) {
                    // Ignore close error
                }
            }
        }

        return resultBuilder.toString();
    }

    public static void main(String[] args) {
        if (args.length > 0 && args.length % 2 == 0) {
            System.out.println("Running test cases from command line arguments...");
             for (int i = 0; i < args.length; i += 2) {
                String baseDn = args[i];
                String username = args[i+1];
                System.out.println("--- Test Case " + (i/2 + 1) + ": baseDn=" + baseDn + ", username=" + username + " ---");
                String result = searchLdapUser(baseDn, username);
                System.out.println(result);
            }
        } else {
            System.out.println("Running predefined test cases...");
            // Test Case 1: A standard valid user search
            System.out.println("--- Test Case 1: Find user 'jdoe' in 'dc=example,dc=com' ---");
            System.out.println(searchLdapUser("dc=example,dc=com", "jdoe"));

            // Test Case 2: Another valid user
            System.out.println("--- Test Case 2: Find user 'asmith' in 'dc=example,dc=com' ---");
            System.out.println(searchLdapUser("dc=example,dc=com", "asmith"));

            // Test Case 3: A user that does not exist
            System.out.println("--- Test Case 3: Find non-existent user 'nobody' ---");
            System.out.println(searchLdapUser("dc=example,dc=com", "nobody"));

            // Test Case 4: A search in a non-existent base DN
            System.out.println("--- Test Case 4: Search in a non-existent base DN ---");
            System.out.println(searchLdapUser("dc=nonexistent,dc=org", "jdoe"));

            // Test Case 5: A user in a different, valid base DN
            System.out.println("--- Test Case 5: Find user 'bcarter' in 'dc=test,dc=org' ---");
            System.out.println(searchLdapUser("dc=test,dc=org", "bcarter"));

            System.out.println("\nUsage for custom tests: java Task68 <baseDn1> <user1> <baseDn2> <user2> ...");
            System.out.println("Example: java Task68 dc=example,dc=com jdoe dc=example,dc=com nonexistant");
        }
    }
}