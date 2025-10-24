import javax.naming.InitialContext;
import javax.naming.NamingException;

/**
 * A class to perform secure JNDI lookups.
 */
public class Task107 {

    /**
     * Performs a JNDI lookup with strict validation to prevent security vulnerabilities.
     * This method only allows lookups within the 'java:' JNDI context to mitigate
     * against remote code execution vulnerabilities like Log4Shell.
     *
     * @param jndiName The JNDI name of the resource to look up.
     * @return The looked-up object, or null if the lookup fails or is blocked.
     */
    public static Object performJndiLookup(String jndiName) {
        // Rule #6: Validate all input.
        // Restrict lookups to a safe context to prevent remote code execution.
        if (jndiName == null || jndiName.trim().isEmpty() || !jndiName.startsWith("java:")) {
            System.err.println("Error: Invalid or insecure JNDI name provided: " + jndiName);
            System.err.println("Lookups are restricted to the 'java:' context for security.");
            return null;
        }

        System.out.println("Attempting to look up secure JNDI name: " + jndiName);

        // Rule #10: Use try-with-resources for automatic resource management of InitialContext.
        try (InitialContext context = new InitialContext()) {
            // In a real Java EE application, the context would be configured by the container.
            // In a standalone app, this will likely fail unless a JNDI provider is configured.
            Object lookedUpObject = context.lookup(jndiName);
            System.out.println("Lookup successful for " + jndiName);
            return lookedUpObject;
        } catch (NamingException e) {
            // Rule #10: Ensure that all exceptions are caught and handled appropriately.
            System.err.println("JNDI lookup failed for name '" + jndiName + "': " + e.getMessage());
            return null;
        } catch (Exception e) {
            // Catching generic Exception for any other unexpected runtime issues.
            System.err.println("An unexpected error occurred during JNDI lookup for '" + jndiName + "': " + e.getMessage());
            return null;
        }
    }

    /**
     * Main method with test cases for the JNDI lookup function.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // Note: In a standalone Java application without a JNDI provider configured,
        // these lookups are expected to fail with a "NameNotFoundException" because
        // the `java:` namespace is typically managed by an application server.
        // The key purpose of this demonstration is to show the *security validation*
        // of the JNDI name before the lookup is even attempted.

        System.out.println("--- Test Case 1: Safe JNDI Name (Java EE standard) ---");
        Object result1 = performJndiLookup("java:comp/env/jdbc/myDataSource");
        System.out.println("Result: " + (result1 != null ? result1.toString() : "Lookup Failed as Expected"));
        System.out.println();

        System.out.println("--- Test Case 2: Another Safe JNDI Name ---");
        Object result2 = performJndiLookup("java:global/ejb/myBean");
        System.out.println("Result: " + (result2 != null ? result2.toString() : "Lookup Failed as Expected"));
        System.out.println();

        System.out.println("--- Test Case 3: Malicious LDAP Lookup (should be blocked) ---");
        Object result3 = performJndiLookup("ldap://malicious.server.com/resource");
        System.out.println("Result: " + (result3 != null ? result3.toString() : "Blocked by Validation"));
        System.out.println();

        System.out.println("--- Test Case 4: Malicious RMI Lookup (should be blocked) ---");
        Object result4 = performJndiLookup("rmi://another.evil.server/exploit");
        System.out.println("Result: " + (result4 != null ? result4.toString() : "Blocked by Validation"));
        System.out.println();

        System.out.println("--- Test Case 5: Null Input (should be blocked) ---");
        Object result5 = performJndiLookup(null);
        System.out.println("Result: " + (result5 != null ? result5.toString() : "Blocked by Validation"));
        System.out.println();
    }
}