public class Task140 {

    /**
     * Performs a simple arithmetic operation.
     *
     * @param num1     The first number.
     * @param num2     The second number.
     * @param operator The operator character (+, -, *, /).
     * @return The result of the operation.
     * @throws IllegalArgumentException if the operator is invalid or division by zero occurs.
     */
    public static double calculate(double num1, double num2, char operator) {
        double result;
        switch (operator) {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;
            case '*':
                result = num1 * num2;
                break;
            case '/':
                if (num2 == 0) {
                    throw new IllegalArgumentException("Error: Division by zero is not allowed.");
                }
                result = num1 / num2;
                break;
            default:
                throw new IllegalArgumentException("Error: Invalid operator. Only +, -, *, / are supported.");
        }
        return result;
    }

    public static void main(String[] args) {
        // Test cases
        double[] nums1 = {10.0, 20.5, 7.0, 15.0, 10.0};
        double[] nums2 = {5.0, 4.5, 6.0, 4.0, 0.0};
        char[] ops = {'+', '-', '*', '/', '/'};

        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < 5; i++) {
            try {
                double result = calculate(nums1[i], nums2[i], ops[i]);
                System.out.printf("Test Case %d: %.2f %c %.2f = %.2f\n", i + 1, nums1[i], ops[i], nums2[i], result);
            } catch (IllegalArgumentException e) {
                System.out.printf("Test Case %d: %.2f %c %.2f -> %s\n", i + 1, nums1[i], ops[i], nums2[i], e.getMessage());
            }
        }
    }
}