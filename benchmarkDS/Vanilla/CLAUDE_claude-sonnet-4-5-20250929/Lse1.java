
public class Lse1 {
    public static void copyBytes(byte[] destination, byte[] source, int numBytes) {
        if (destination == null || source == null) {
            return;
        }
        
        int bytesToCopy = Math.min(numBytes, Math.min(destination.length, source.length));
        
        for (int i = 0; i < bytesToCopy; i++) {
            destination[i] = source[i];
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy
        byte[] dest1 = new byte[10];
        byte[] src1 = {1, 2, 3, 4, 5};
        copyBytes(dest1, src1, 5);
        System.out.print("Test 1: ");
        for (int i = 0; i < 5; i++) System.out.print(dest1[i] + " ");
        System.out.println();
        
        // Test case 2: Copy fewer bytes than source
        byte[] dest2 = new byte[10];
        byte[] src2 = {10, 20, 30, 40, 50};
        copyBytes(dest2, src2, 3);
        System.out.print("Test 2: ");
        for (int i = 0; i < 3; i++) System.out.print(dest2[i] + " ");
        System.out.println();
        
        // Test case 3: Copy with larger numBytes than source
        byte[] dest3 = new byte[10];
        byte[] src3 = {7, 8, 9};
        copyBytes(dest3, src3, 10);
        System.out.print("Test 3: ");
        for (int i = 0; i < 3; i++) System.out.print(dest3[i] + " ");
        System.out.println();
        
        // Test case 4: Copy zero bytes
        byte[] dest4 = new byte[5];
        byte[] src4 = {11, 12, 13};
        copyBytes(dest4, src4, 0);
        System.out.print("Test 4: ");
        for (int i = 0; i < 5; i++) System.out.print(dest4[i] + " ");
        System.out.println();
        
        // Test case 5: Copy all bytes
        byte[] dest5 = new byte[5];
        byte[] src5 = {100, 101, 102, 103, 104};
        copyBytes(dest5, src5, 5);
        System.out.print("Test 5: ");
        for (int i = 0; i < 5; i++) System.out.print(dest5[i] + " ");
        System.out.println();
    }
}
