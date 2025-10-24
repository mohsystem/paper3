
import java.io.*;
import java.util.*;

class Task131 {
    // Deserialize from byte array
    public static Object deserializeFromBytes(byte[] data) throws IOException, ClassNotFoundException {
        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        ObjectInputStream ois = new ObjectInputStream(bis);
        Object obj = ois.readObject();
        ois.close();
        return obj;
    }
    
    // Deserialize from file
    public static Object deserializeFromFile(String filename) throws IOException, ClassNotFoundException {
        FileInputStream fis = new FileInputStream(filename);
        ObjectInputStream ois = new ObjectInputStream(fis);
        Object obj = ois.readObject();
        ois.close();
        return obj;
    }
    
    // Helper method to serialize an object (for testing)
    public static byte[] serializeToBytes(Object obj) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(bos);
        oos.writeObject(obj);
        oos.close();
        return bos.toByteArray();
    }
    
    public static void main(String[] args) {
        try {
            // Test Case 1: Serialize and deserialize a String
            System.out.println("Test Case 1: String");
            String str1 = "Hello, World!";
            byte[] serialized1 = serializeToBytes(str1);
            String deserialized1 = (String) deserializeFromBytes(serialized1);
            System.out.println("Original: " + str1);
            System.out.println("Deserialized: " + deserialized1);
            System.out.println();
            
            // Test Case 2: Serialize and deserialize an Integer
            System.out.println("Test Case 2: Integer");
            Integer num = 42;
            byte[] serialized2 = serializeToBytes(num);
            Integer deserialized2 = (Integer) deserializeFromBytes(serialized2);
            System.out.println("Original: " + num);
            System.out.println("Deserialized: " + deserialized2);
            System.out.println();
            
            // Test Case 3: Serialize and deserialize an ArrayList
            System.out.println("Test Case 3: ArrayList");
            ArrayList<String> list = new ArrayList<>(Arrays.asList("Apple", "Banana", "Cherry"));
            byte[] serialized3 = serializeToBytes(list);
            @SuppressWarnings("unchecked")
            ArrayList<String> deserialized3 = (ArrayList<String>) deserializeFromBytes(serialized3);
            System.out.println("Original: " + list);
            System.out.println("Deserialized: " + deserialized3);
            System.out.println();
            
            // Test Case 4: Serialize and deserialize a HashMap
            System.out.println("Test Case 4: HashMap");
            HashMap<String, Integer> map = new HashMap<>();
            map.put("One", 1);
            map.put("Two", 2);
            map.put("Three", 3);
            byte[] serialized4 = serializeToBytes(map);
            @SuppressWarnings("unchecked")
            HashMap<String, Integer> deserialized4 = (HashMap<String, Integer>) deserializeFromBytes(serialized4);
            System.out.println("Original: " + map);
            System.out.println("Deserialized: " + deserialized4);
            System.out.println();
            
            // Test Case 5: Serialize and deserialize a Double array
            System.out.println("Test Case 5: Double Array");
            Double[] arr = {1.1, 2.2, 3.3, 4.4, 5.5};
            byte[] serialized5 = serializeToBytes(arr);
            Double[] deserialized5 = (Double[]) deserializeFromBytes(serialized5);
            System.out.println("Original: " + Arrays.toString(arr));
            System.out.println("Deserialized: " + Arrays.toString(deserialized5));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
