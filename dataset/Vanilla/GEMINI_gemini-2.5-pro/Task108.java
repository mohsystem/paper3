import java.rmi.Naming;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.server.UnicastRemoteObject;

// 1. Define the remote interface
interface RemoteCalculator extends Remote {
    double add(double a, double b) throws RemoteException;
    double subtract(double a, double b) throws RemoteException;
    double multiply(double a, double b) throws RemoteException;
    double divide(double a, double b) throws RemoteException;
}

// 2. Implement the remote object
class RemoteCalculatorImpl extends UnicastRemoteObject implements RemoteCalculator {
    public RemoteCalculatorImpl() throws RemoteException {
        super();
    }

    @Override
    public double add(double a, double b) throws RemoteException {
        return a + b;
    }

    @Override
    public double subtract(double a, double b) throws RemoteException {
        return a - b;
    }

    @Override
    public double multiply(double a, double b) throws RemoteException {
        return a * b;
    }

    @Override
    public double divide(double a, double b) throws RemoteException {
        if (b == 0) {
            throw new RemoteException("Error: Division by zero.");
        }
        return a / b;
    }
}


public class Task108 {

    // 3. Server main method
    private static void startServer() {
        try {
            // Create and export a remote object
            RemoteCalculator calculator = new RemoteCalculatorImpl();

            // Start the RMI registry on port 1099
            LocateRegistry.createRegistry(1099);

            // Bind the remote object's stub in the registry
            Naming.rebind("//localhost/CalculatorService", calculator);

            System.out.println("Calculator Server is ready.");

        } catch (Exception e) {
            System.err.println("Server exception: " + e.toString());
            e.printStackTrace();
        }
    }

    // 4. Client main method with test cases
    private static void startClient() {
        try {
            // Look up the remote object
            RemoteCalculator calculator = (RemoteCalculator) Naming.lookup("//localhost/CalculatorService");

            System.out.println("Client connected to Calculator Server.");
            System.out.println("--- Running Test Cases ---");

            // Test Case 1: Addition
            double a = 5, b = 3;
            double resultAdd = calculator.add(a, b);
            System.out.println("Test 1: " + a + " + " + b + " = " + resultAdd);

            // Test Case 2: Subtraction
            a = 10; b = 4;
            double resultSubtract = calculator.subtract(a, b);
            System.out.println("Test 2: " + a + " - " + b + " = " + resultSubtract);

            // Test Case 3: Multiplication
            a = 7; b = 6;
            double resultMultiply = calculator.multiply(a, b);
            System.out.println("Test 3: " + a + " * " + b + " = " + resultMultiply);

            // Test Case 4: Division
            a = 20; b = 5;
            double resultDivide = calculator.divide(a, b);
            System.out.println("Test 4: " + a + " / " + b + " = " + resultDivide);

            // Test Case 5: Division by zero
            try {
                a = 10; b = 0;
                System.out.print("Test 5: " + a + " / " + b + " -> ");
                calculator.divide(a, b);
            } catch (RemoteException e) {
                System.out.println("Successfully caught expected exception: " + e.getMessage());
            }

        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }

    /**
     * Main method to launch either the server or the client.
     * --- How to Run ---
     * 1. Compile: javac Task108.java
     * 2. Start Server in one terminal: java Task108 server
     * 3. Start Client in another terminal: java Task108 client
     */
    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.println("Usage: java Task108 <server|client>");
            return;
        }

        if (args[0].equalsIgnoreCase("server")) {
            startServer();
        } else if (args[0].equalsIgnoreCase("client")) {
            startClient();
        } else {
            System.out.println("Invalid mode. Use 'server' or 'client'.");
        }
    }
}