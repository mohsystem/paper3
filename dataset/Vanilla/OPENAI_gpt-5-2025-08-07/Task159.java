import java.util.*;

public class Task159 {
    // Dynamically allocates an int buffer, fills it, and returns the value at the given index.
    // Returns -1 for invalid size, allocation failure (conceptual), or out-of-bounds index.
    public static int readFromBuffer(int size, int index, int fillBase) {
        if (size <= 0) return -1;
        int[] buffer = new int[size]; // dynamic allocation in Java
        for (int i = 0; i < size; i++) {
            buffer[i] = fillBase + i;
        }
        if (index < 0 || index >= size) return -1;
        return buffer[index];
    }

    public static void main(String[] args) {
        int size = 5;
        int fillBase = 100;
        int[] testIndices = {0, 4, 5, -1, 2};

        for (int idx : testIndices) {
            int result = readFromBuffer(size, idx, fillBase);
            System.out.println("Index " + idx + " -> " + result);
        }
    }
}