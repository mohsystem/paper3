import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.rmi.registry.LocateRegistry;
import java.util.Hashtable;
import java.util.ArrayList;
import java.util.Arrays;

// Note: This example uses the RMI registry as a simple JNDI service provider.
// It must be run with the appropriate permissions.
// In a real-world scenario, you would typically look up resources managed by an
// application server (like a DataSource) rather than binding them yourself in the code.
public class Task107 {

    /**
     * Performs a JNDI lookup for a given name.
     *
     * @param jndiName The JNDI name of the object to look up.
     * @return The object retrieved from the JNDI context.
     * @throws NamingException if a naming error is encountered.
     */
    public static Object performJndiLookup(String jndiName) throws NamingException {
        // Set up the environment for creating the initial context
        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.rmi.registry.RegistryContextFactory");
        env.put(Context.PROVIDER_URL, "rmi://localhost:1099");

        InitialContext ctx = null;
        try {
            ctx = new InitialContext(env);
            System.out.println("Performing lookup for: " + jndiName);
            Object lookedUpObject = ctx.lookup(jndiName);
            return lookedUpObject;
        } finally {
            if (ctx != null) {
                try {
                    ctx.close();
                } catch (NamingException e) {
                    // In a real app, you might log this. For the example, we'll print it.
                    System.err.println("Error closing JNDI context: " + e.getMessage());
                }
            }
        }
    }

    // Helper method to setup a local RMI registry and bind objects for testing
    private static void setupJndiTestEnvironment() throws Exception {
        // 1. Create and start the RMI registry on port 1099
        try {
            LocateRegistry.createRegistry(1099);
            System.out.println("RMI registry started on port 1099.");
        } catch (java.rmi.server.ExportException e) {
            System.out.println("RMI registry seems to be already running.");
        }

        // 2. Get the JNDI initial context for binding
        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.rmi.registry.RegistryContextFactory");
        env.put(Context.PROVIDER_URL, "rmi://localhost:1099");
        InitialContext ctx = new InitialContext(env);

        // 3. Bind objects for test cases using rebind to avoid errors on subsequent runs
        System.out.println("Binding objects to JNDI context...");
        ctx.rebind("jndi/myString", "Hello from JNDI!");
        ctx.rebind("jndi/myInteger", 42);
        ctx.rebind("jndi/myList", new ArrayList<String>(Arrays.asList("Apple", "Banana", "Cherry")));
        System.out.println("Objects bound successfully.");
        
        ctx.close();
    }


    public static void main(String[] args) {
        try {
            // Setup the RMI registry and bind some test objects
            setupJndiTestEnvironment();
            System.out.println("\n--- Starting JNDI Lookup Test Cases ---");
        } catch (Exception e) {
            System.err.println("Failed to set up JNDI test environment: " + e.getMessage());
            e.printStackTrace();
            return;
        }

        // Define test cases
        String[] testJndiNames = {
            "jndi/myString",       // Test Case 1: Lookup a String
            "jndi/myInteger",      // Test Case 2: Lookup an Integer
            "jndi/myList",         // Test Case 3: Lookup a Serializable List
            "jndi/nonExistent",    // Test Case 4: Lookup a non-existent object (expected to fail)
            "jndi/myString"        // Test Case 5: Lookup the same String again
        };

        // Run test cases
        for (int i = 0; i < testJndiNames.length; i++) {
            String name = testJndiNames[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            try {
                Object result = performJndiLookup(name);
                System.out.println("Lookup successful for '" + name + "'");
                System.out.println("  - Type: " + result.getClass().getName());
                System.out.println("  - Value: " + result.toString());
            } catch (NamingException e) {
                System.err.println("Lookup failed for '" + name + "' as expected: " + e.getClass().getName() + ": " + e.getMessage());
            }
        }
        
        // The RMI registry thread is non-daemon and will keep the JVM alive.
        // For this self-contained example, we explicitly exit.
        System.exit(0);
    }
}