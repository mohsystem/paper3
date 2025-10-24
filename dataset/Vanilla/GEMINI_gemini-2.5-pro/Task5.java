public class Task5 {

    public static int persistence(long num) {
        int count = 0;
        while (num >= 10) {
            count++;
            long product = 1;
            long tempNum = num;
            while (tempNum > 0) {
                product *= tempNum % 10;
                tempNum /= 10;
            }
            num = product;
        }
        return count;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Input: 39, Output: " + persistence(39)); // Expected: 3
        System.out.println("Input: 999, Output: " + persistence(999)); // Expected: 4
        System.out.println("Input: 4, Output: " + persistence(4)); // Expected: 0
        System.out.println("Input: 25, Output: " + persistence(25)); // Expected: 2
        System.out.println("Input: 679, Output: " + persistence(679)); // Expected: 5
    }
}