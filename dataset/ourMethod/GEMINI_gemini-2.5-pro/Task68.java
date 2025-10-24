import javax.naming.*;
import javax.naming.directory.*;
import java.util.Hashtable;
import java.util.regex.Pattern;

/**
 * Note: This program requires a dependency on the JNDI LDAP provider.
 * This is typically included in the Java SE runtime. To run, compile with
 * javac Task68.java and run with java Task68 [username@domain.com]
 * An LDAP server must be running on localhost:389 for this to work.
 */
public class Task68 {

    /**
     * Extracts user info from an LDAP directory based on user input.
     * @param userInput A string in the format "username@domain.com".
     * @return A string containing the search results or an error message.
     */
    public static String findUserInLdap(String userInput) {
        if (userInput == null || userInput.isEmpty()) {
            return "Error: Input cannot be null or empty.";
        }

        // 1. Extract username and domain components
        String[] parts = userInput.split("@", 2);
        if (parts.length != 2 || parts[0].isEmpty() || parts[1].isEmpty()) {
            return "Error: Invalid input format. Expected 'username@domain.com'.";
        }
        String username = parts[0];
        String domain = parts[1];

        // 3. Validate inputs to prevent unexpected behavior.
        // Allow common characters in domains and usernames.
        if (!Pattern.matches("^[a-zA-Z0-9.-]+$", domain)) {
            return "Error: Domain contains invalid characters.";
        }
        if (!Pattern.matches("^[a-zA-Z0-9_.-]+$", username)) {
            return "Error: Username contains invalid characters.";
        }

        // 2. Construct Base DN
        String baseDn = "dc=" + domain.replace(".", ",dc=");

        // 4. Sanitize username for LDAP filter (prevents LDAP Injection)
        String escapedUsername = escapeLdapFilter(username);
        String searchFilter = "(uid=" + escapedUsername + ")";

        // 5. Connect and search LDAP
        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        env.put(Context.PROVIDER_URL, "ldap://localhost:389");
        // For production, use LDAPS: "ldaps://your-ldap-server:636"
        // And configure SSL/TLS properly, e.g., System.setProperty("javax.net.ssl.trustStore", ...);

        StringBuilder resultBuilder = new StringBuilder();
        
        // Use try-with-resources to ensure the context is closed
        try (DirContext ctx = new InitialDirContext(env)) {
            SearchControls searchControls = new SearchControls();
            searchControls.setSearchScope(SearchControls.SUBTREE_SCOPE);

            resultBuilder.append("Searching for user '").append(username).append("' in '").append(baseDn).append("'\n");
            resultBuilder.append("Using filter: ").append(searchFilter).append("\n");

            NamingEnumeration<SearchResult> results = ctx.search(baseDn, searchFilter, searchControls);

            if (!results.hasMore()) {
                resultBuilder.append("User not found.\n");
            } else {
                while (results.hasMore()) {
                    SearchResult searchResult = results.next();
                    resultBuilder.append("Found entry: ").append(searchResult.getNameInNamespace()).append("\n");
                    Attributes attributes = searchResult.getAttributes();
                    NamingEnumeration<? extends Attribute> allAttributes = attributes.getAll();
                    while(allAttributes.hasMore()){
                        Attribute attr = allAttributes.next();
                        resultBuilder.append("  ").append(attr.getID()).append(": ").append(attr.get()).append("\n");
                    }
                }
            }
        } catch (AuthenticationException e) {
            return "LDAP Authentication failed: " + e.getMessage();
        } catch (CommunicationException e) {
            return "Error: Could not connect to LDAP server at localhost:389. Is it running?";
        } catch (NamingException e) {
            return "LDAP search error: " + e.toString();
        }

        return resultBuilder.toString();
    }

    /**
     * Escapes special characters in a string for use in an LDAP filter
     * according to RFC 4515. This is crucial to prevent LDAP injection.
     * @param filter The raw string to escape.
     * @return The escaped string safe for use in an LDAP filter.
     */
    private static String escapeLdapFilter(String filter) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < filter.length(); i++) {
            char cur = filter.charAt(i);
            switch (cur) {
                case '\\': sb.append("\\5c"); break;
                case '(':  sb.append("\\28"); break;
                case ')':  sb.append("\\29"); break;
                case '*':  sb.append("\\2a"); break;
                case '\0': sb.append("\\00"); break;
                default:   sb.append(cur);
            }
        }
        return sb.toString();
    }
    
    public static void main(String[] args) {
        if (args.length > 0) {
            System.out.println("--- Running with provided argument ---");
            String result = findUserInLdap(args[0]);
            System.out.println(result);
        } else {
            System.out.println("--- Running with pre-defined test cases ---");
            System.out.println("NOTE: These tests require a local LDAP server populated with sample data.");
            
            String[] testCases = {
                "jdoe@example.com",           // Normal valid user
                "admin@example.com",          // Another valid user
                "test.user@sub.example.com",  // User in a subdomain
                "*@example.com",              // Potential injection attempt, should be escaped
                "jdoe)(uid=*))(|(uid=*@example.com", // Advanced injection attempt
            };

            for (int i = 0; i < testCases.length; i++) {
                System.out.println("\n--- Test Case " + (i + 1) + ": " + testCases[i] + " ---");
                String result = findUserInLdap(testCases[i]);
                System.out.println(result);
            }
        }
    }
}