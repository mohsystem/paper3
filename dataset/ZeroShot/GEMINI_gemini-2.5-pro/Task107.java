import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.HashSet;
import java.util.Properties;
import java.util.Set;

public class Task107 {

    /**
     * A predefined allowlist of JNDI names that the application is permitted to look up.
     * This is a critical security control to prevent Log4Shell-style vulnerabilities.
     */
    private static final Set<String> JNDI_ALLOWLIST = new HashSet<>();
    
    static {
        // Populate the allowlist with known, safe JNDI names
        JNDI_ALLOWLIST.add("rmi://localhost:1099/mySafeObject");
        JNDI_ALLOWLIST.add("rmi://localhost:1099/anotherSafeObject");
    }

    /**
     * Performs a secure JNDI lookup.
     * Security is enforced by:
     * 1. Validating the JNDI name against a strict allowlist.
     * 2. Disabling remote codebase loading (`trustURLCodebase`), which is the primary
     *    vector for JNDI injection attacks. This is the default in modern JDKs, but
     *    setting it explicitly provides defense-in-depth.
     *
     * @param jndiName The JNDI name to look up.
     * @return The retrieved object, or null if the lookup is blocked or fails.
     */
    public static Object secureJndiLookup(String jndiName) {
        // 1. Security Control: Validate the name against the allowlist.
        if (jndiName == null || !JNDI_ALLOWLIST.contains(jndiName)) {
            System.err.println("SECURITY ALERT: JNDI lookup blocked. Name is not in the allowlist: " + jndiName);
            return null;
        }

        try {
            // 2. Security Control: Set properties to disable insecure JNDI features.
            Properties props = new Properties();
            // These properties prevent JNDI from loading classes from a remote URL,
            // mitigating RCE vulnerabilities (e.g., Log4Shell).
            props.put("com.sun.jndi.rmi.object.trustURLCodebase", "false");
            props.put("com.sun.jndi.ldap.object.trustURLCodebase", "false");

            InitialContext ctx = new InitialContext(props);
            
            System.out.println("Performing secure JNDI lookup for: " + jndiName);
            Object obj = ctx.lookup(jndiName);
            ctx.close();
            return obj;

        } catch (NamingException e) {
            System.err.println("JNDI lookup failed for '" + jndiName + "'. Reason: " + e.getMessage());
            return null;
        } catch (Exception e) {
            System.err.println("An unexpected error occurred during lookup for '" + jndiName + "': " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        Registry registry = null;
        try {
            // Setup: Start a local RMI registry for testing purposes.
            // In a real application, this would be part of the application server.
            registry = LocateRegistry.createRegistry(1099);
            registry.bind("mySafeObject", "This is a safe string object.");
            registry.bind("anotherSafeObject", 12345); // Bind another object type
            System.out.println("RMI Registry started and test objects are bound.");
            
            // This object is intentionally NOT in the JNDI_ALLOWLIST to test the security control.
            registry.bind("unlistedObject", "This object is not in the allowlist.");

            // --- Running Test Cases ---
            System.out.println("\n--- Running Test Cases ---");

            // Test Case 1: Valid and allowed name
            System.out.println("\n[Test Case 1: Valid Name]");
            Object result1 = secureJndiLookup("rmi://localhost:1099/mySafeObject");
            if (result1 != null) {
                System.out.println("Lookup successful. Object: \"" + result1 + "\", Type: " + result1.getClass().getName());
            }

            // Test Case 2: Another valid and allowed name
            System.out.println("\n[Test Case 2: Another Valid Name]");
            Object result2 = secureJndiLookup("rmi://localhost:1099/anotherSafeObject");
            if (result2 != null) {
                System.out.println("Lookup successful. Object: " + result2 + ", Type: " + result2.getClass().getName());
            }

            // Test Case 3: A name that exists but is NOT in the allowlist
            System.out.println("\n[Test Case 3: Name Not In Allowlist]");
            Object result3 = secureJndiLookup("rmi://localhost:1099/unlistedObject");
            if (result3 == null) {
                System.out.println("Lookup correctly blocked by the allowlist as expected.");
            }

            // Test Case 4: A potentially malicious name using a different protocol
            System.out.println("\n[Test Case 4: Potentially Malicious Name]");
            Object result4 = secureJndiLookup("ldap://malicious-server.com/exploit");
            if (result4 == null) {
                System.out.println("Lookup correctly blocked by the allowlist as expected.");
            }

            // Test Case 5: Null input
            System.out.println("\n[Test Case 5: Null Input]");
            Object result5 = secureJndiLookup(null);
            if (result5 == null) {
                System.out.println("Lookup correctly blocked for null input as expected.");
            }

        } catch (Exception e) {
            System.err.println("An error occurred during the test setup: " + e.getMessage());
            // In case the port is already in use, which can happen when re-running the test
            if (e instanceof java.rmi.server.ExportException) {
                System.err.println("Hint: Port 1099 might be in use. Please ensure no other RMI registry is running on it.");
            }
        } finally {
            // Clean up: For this demo, we let the program exit.
            // In a long-running app, the registry would be managed properly.
            System.out.println("\n--- Test Cases Complete ---");
        }
    }
}