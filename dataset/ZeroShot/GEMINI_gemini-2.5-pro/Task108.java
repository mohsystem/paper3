/*
================================================================================
Instructions for Java (RMI over SSL/TLS):
1.  This code must be compiled and run from the command line.
2.  Create a keystore and truststore for SSL. Execute the following command in your terminal:
    keytool -genkeypair -alias task108 -keyalg RSA -keysize 2048 -storetype JKS -keystore keystore.jks -validity 365
    (When prompted, provide a password, e.g., "password", and answer the questions. The first/last name should be "localhost".)

3.  Compile the Java file:
    javac Task108.java

4.  Start the RMI registry (in a separate terminal, from the directory containing the .class file):
    rmiregistry 1099
    (You might need to adjust your CLASSPATH or run it from the right directory)

5.  Start the server (in another terminal):
    java -Djavax.net.ssl.keyStore=keystore.jks -Djavax.net.ssl.keyStorePassword=password Task108 server

6.  Run the client to execute test cases (in another terminal):
    java -Djavax.net.ssl.trustStore=keystore.jks -Djavax.net.ssl.trustStorePassword=password Task108 client
================================================================================
*/
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.rmi.server.RMIClientSocketFactory;
import java.rmi.server.RMIServerSocketFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.rmi.ssl.SslRMIClientSocketFactory;
import javax.rmi.ssl.SslRMIServerSocketFactory;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;

// 1. Define the remote interface
interface RemoteObjectManager extends Remote {
    String get(String token, String key) throws RemoteException, SecurityException;
    void put(String token, String key, String value) throws RemoteException, SecurityException;
    String delete(String token, String key) throws RemoteException, SecurityException;
}

// 2. Implement the remote interface
class RemoteObjectManagerImpl extends UnicastRemoteObject implements RemoteObjectManager {
    private final Map<String, String> dataStore = new ConcurrentHashMap<>();
    private static final String SECRET_TOKEN = "secret-java-token-123";

    protected RemoteObjectManagerImpl(RMIClientSocketFactory csf, RMIServerSocketFactory ssf) throws RemoteException {
        super(0, csf, ssf);
    }

    private void checkToken(String token) throws SecurityException {
        if (token == null || !token.equals(SECRET_TOKEN)) {
            throw new SecurityException("Invalid or missing authentication token.");
        }
    }

    @Override
    public String get(String token, String key) throws RemoteException, SecurityException {
        checkToken(token);
        return dataStore.get(key);
    }

    @Override
    public void put(String token, String key, String value) throws RemoteException, SecurityException {
        checkToken(token);
        dataStore.put(key, value);
    }

    @Override
    public String delete(String token, String key) throws RemoteException, SecurityException {
        checkToken(token);
        return dataStore.remove(key);
    }
}

// 3. Main class to start server or run client tests
public class Task108 {

    private static final int RMI_PORT = 1099;
    private static final String BINDING_NAME = "RemoteObjectManager";
    private static final String SECRET_TOKEN = "secret-java-token-123";

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Usage: java Task108 <server|client>");
            System.exit(1);
        }

        if (args[0].equalsIgnoreCase("server")) {
            startServer();
        } else if (args[0].equalsIgnoreCase("client")) {
            runClientTests();
        } else {
            System.err.println("Invalid mode. Use 'server' or 'client'.");
            System.exit(1);
        }
    }

    private static void startServer() {
        try {
            System.out.println("Starting secure RMI server...");

            // Create secure socket factories
            RMIClientSocketFactory csf = new SslRMIClientSocketFactory();
            RMIServerSocketFactory ssf = new SslRMIServerSocketFactory();

            // Create the remote object
            RemoteObjectManagerImpl obj = new RemoteObjectManagerImpl(csf, ssf);

            // Create and bind to the RMI registry
            Registry registry = LocateRegistry.createRegistry(RMI_PORT, csf, ssf);
            registry.rebind(BINDING_NAME, obj);

            System.out.println("Server is ready. Bound as '" + BINDING_NAME + "'.");
        } catch (Exception e) {
            System.err.println("Server exception: " + e.toString());
            e.printStackTrace();
        }
    }

    private static void runClientTests() {
        try {
            System.out.println("Running client tests...");
            Registry registry = LocateRegistry.getRegistry("localhost", RMI_PORT, new SslRMIClientSocketFactory());
            RemoteObjectManager stub = (RemoteObjectManager) registry.lookup(BINDING_NAME);
            System.out.println("Successfully looked up remote object.");

            // Test Case 1: Put a new key-value pair
            System.out.println("\n--- Test Case 1: Put new item ---");
            stub.put(SECRET_TOKEN, "user1", "data123");
            System.out.println("put('user1', 'data123') -> OK");

            // Test Case 2: Get the value for the new key
            System.out.println("\n--- Test Case 2: Get new item ---");
            String value = stub.get(SECRET_TOKEN, "user1");
            System.out.println("get('user1') -> " + value);
            assert "data123".equals(value);

            // Test Case 3: Update the value for an existing key
            System.out.println("\n--- Test Case 3: Update existing item ---");
            stub.put(SECRET_TOKEN, "user1", "data456");
            System.out.println("put('user1', 'data456') -> OK");

            // Test Case 4: Get the updated value
            System.out.println("\n--- Test Case 4: Get updated item ---");
            value = stub.get(SECRET_TOKEN, "user1");
            System.out.println("get('user1') -> " + value);
            assert "data456".equals(value);

            // Test Case 5: Delete the key
            System.out.println("\n--- Test Case 5: Delete item ---");
            String deletedValue = stub.delete(SECRET_TOKEN, "user1");
            System.out.println("delete('user1') -> " + deletedValue);
            assert "data456".equals(deletedValue);
            value = stub.get(SECRET_TOKEN, "user1");
            System.out.println("get('user1') after delete -> " + value);
            assert value == null;
            
            // Test Case 6: Authentication Failure
            System.out.println("\n--- Test Case 6: Authentication Failure ---");
            try {
                stub.get("wrong-token", "user1");
            } catch (SecurityException e) {
                System.out.println("Caught expected security exception: " + e.getMessage());
            }

            System.out.println("\nAll client tests completed.");

        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }
}