import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Set;

public class Task107 {

    // A whitelist of allowed JNDI names to prevent JNDI Injection attacks.
    // In a real application, this would be populated from a secure configuration.
    private static final Set<String> ALLOWED_JNDI_NAMES = new HashSet<>();

    static {
        // Populate the whitelist with known, safe JNDI names.
        ALLOWED_JNDI_NAMES.add("java:comp/env/jdbc/myDataSource");
        ALLOWED_JNDI_NAMES.add("java:comp/env/jms/myQueue");
        ALLOWED_JNDI_NAMES.add("java:comp/env/ejb/myBean");
    }

    /**
     * Performs a JNDI lookup in a secure manner.
     * It validates the JNDI name against a pre-approved whitelist before performing the lookup.
     *
     * @param jndiName The JNDI name of the resource to look up.
     * @return The looked-up object, or null if the lookup fails or is disallowed.
     */
    public static Object performJndiLookup(String jndiName) {
        // 1. Input Validation: Check against a whitelist of allowed names.
        if (jndiName == null || !ALLOWED_JNDI_NAMES.contains(jndiName)) {
            System.err.println("SECURITY ALERT: JNDI lookup blocked for untrusted name: " + jndiName);
            return null;
        }

        System.out.println("Attempting secure JNDI lookup for whitelisted name: " + jndiName);

        Object lookedUpObject = null;
        try {
            // 2. Perform the lookup only after validation.
            Context ctx = new InitialContext();
            lookedUpObject = ctx.lookup(jndiName);
            System.out.println("Lookup successful for " + jndiName);
        } catch (NamingException e) {
            // This exception is expected in a standard Java SE environment
            // that doesn't have a JNDI provider configured.
            System.err.println("JNDI lookup failed for '" + jndiName + "'. Reason: " + e.getMessage());
            System.err.println("(Note: This is expected if you are not running in a Java EE container or with a configured JNDI provider.)");
        } catch (Exception e) {
            System.err.println("An unexpected error occurred during JNDI lookup for " + jndiName + ": " + e.getMessage());
        }

        return lookedUpObject;
    }

    public static void main(String[] args) {
        System.out.println("--- JNDI Lookup Test Cases ---");
        System.out.println("NOTE: These tests demonstrate the security validation logic. Actual lookups will fail without a configured JNDI context.");

        // Test Case 1: A valid, whitelisted JNDI name.
        System.out.println("\n--- Test Case 1: Whitelisted Name ---");
        performJndiLookup("java:comp/env/jdbc/myDataSource");

        // Test Case 2: Another valid, whitelisted name.
        System.out.println("\n--- Test Case 2: Another Whitelisted Name ---");
        performJndiLookup("java:comp/env/jms/myQueue");
        
        // Test Case 3: A non-whitelisted name (should be blocked).
        System.out.println("\n--- Test Case 3: Non-Whitelisted Name ---");
        performJndiLookup("java:comp/env/jdbc/unapprovedDB");

        // Test Case 4: A potentially malicious JNDI name (should be blocked).
        // This resembles the Log4Shell attack vector.
        System.out.println("\n--- Test Case 4: Malicious-looking Name ---");
        performJndiLookup("ldap://malicious-server.com/exploit");

        // Test Case 5: A null input (should be blocked).
        System.out.println("\n--- Test Case 5: Null Input ---");
        performJndiLookup(null);
    }
}