public class Task23 {

    public static long findNb(long m) {
        long n = 0;
        long remainingVolume = m;
        while (remainingVolume > 0) {
            n++;
            long cubeVolume = n * n * n;
            remainingVolume -= cubeVolume;
        }

        if (remainingVolume == 0) {
            return n;
        } else {
            return -1;
        }
    }

    public static void main(String[] args) {
        // Test cases
        long[] testM = {1071225L, 91716553919377L, 4183059834009L, 24723578342962L, 1L};
        long[] expectedN = {45L, -1L, 2022L, -1L, 1L};

        for (int i = 0; i < testM.length; i++) {
            long result = findNb(testM[i]);
            System.out.println("findNb(" + testM[i] + ") -> " + result +
                               " (Expected: " + expectedN[i] + ")");
        }
    }
}