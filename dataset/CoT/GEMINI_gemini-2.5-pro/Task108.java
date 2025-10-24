import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.rmi.Naming;

public class Task108 {

    // 1. Define the remote interface
    // This interface declares the methods that a client can call remotely.
    // It must extend java.rmi.Remote.
    public interface RemoteCalculator extends Remote {
        int add(int a, int b) throws RemoteException;
        int subtract(int a, int b) throws RemoteException;
        int multiply(int a, int b) throws RemoteException;
        double divide(int a, int b) throws RemoteException;
    }

    // 2. Implement the remote interface
    // This is the server-side implementation of the remote methods.
    // It extends UnicastRemoteObject to handle the RMI communication.
    public static class RemoteCalculatorImpl extends UnicastRemoteObject implements RemoteCalculator {
        public RemoteCalculatorImpl() throws RemoteException {
            super();
        }

        @Override
        public int add(int a, int b) throws RemoteException {
            System.out.println("Server: Adding " + a + " and " + b);
            return a + b;
        }

        @Override
        public int subtract(int a, int b) throws RemoteException {
            System.out.println("Server: Subtracting " + b + " from " + a);
            return a - b;
        }

        @Override
        public int multiply(int a, int b) throws RemoteException {
            System.out.println("Server: Multiplying " + a + " and " + b);
            return a * b;
        }

        @Override
        public double divide(int a, int b) throws RemoteException {
            System.out.println("Server: Dividing " + a + " by " + b);
            // Input validation on the server side
            if (b == 0) {
                throw new RemoteException("Division by zero is not allowed.");
            }
            return (double) a / b;
        }
    }

    // 3. Main method to start server and run client tests
    public static void main(String[] args) {
        String host = "localhost";
        int port = 1099;
        String serviceName = "CalculatorService";
        String bindLocation = "//" + host + ":" + port + "/" + serviceName;
        Registry registry = null;

        try {
            // --- Server Setup ---
            // Create and export a new instance of the remote object.
            RemoteCalculatorImpl serverObject = new RemoteCalculatorImpl();

            // Start the RMI registry on the specified port.
            // This is a directory service for remote objects.
            registry = LocateRegistry.createRegistry(port);
            System.out.println("Server: RMI registry created on port " + port);

            // Bind the remote object's stub in the registry.
            // The Naming.rebind method is used here.
            Naming.rebind(bindLocation, serverObject);
            System.out.println("Server: CalculatorService bound in registry");
            System.out.println("Server is ready.");

            // --- Client Logic ---
            System.out.println("\nClient: Looking up " + bindLocation + "...");
            // Lookup the remote object from the RMI registry.
            // This returns a "stub" that the client can use.
            RemoteCalculator calculator = (RemoteCalculator) Naming.lookup(bindLocation);
            System.out.println("Client: Service found. Running test cases...");

            // --- Test Cases ---
            // Test Case 1: Addition
            int sum = calculator.add(10, 5);
            System.out.println("Client Test 1 (10 + 5): " + sum);

            // Test Case 2: Subtraction
            int difference = calculator.subtract(10, 5);
            System.out.println("Client Test 2 (10 - 5): " + difference);

            // Test Case 3: Multiplication
            int product = calculator.multiply(10, 5);
            System.out.println("Client Test 3 (10 * 5): " + product);

            // Test Case 4: Division
            double quotient = calculator.divide(10, 5);
            System.out.println("Client Test 4 (10 / 5): " + quotient);

            // Test Case 5: Division by zero (error handling)
            try {
                calculator.divide(10, 0);
            } catch (RemoteException e) {
                System.out.println("Client Test 5 (10 / 0): Successfully caught expected exception: " + e.getMessage());
            }

        } catch (Exception e) {
            System.err.println("An error occurred: " + e.toString());
            e.printStackTrace();
        } finally {
            // --- Cleanup ---
            // Unbind the service and shut down the registry
            try {
                if (registry != null) {
                    Naming.unbind(bindLocation);
                    UnicastRemoteObject.unexportObject(registry, true);
                    System.out.println("\nServer: Service unbound and registry shut down.");
                }
            } catch (Exception e) {
                System.err.println("Error during cleanup: " + e.toString());
            }
             // In a real application, you wouldn't exit immediately.
             // But for this single-file example, we exit to stop the RMI server threads.
             System.exit(0);
        }
    }
}