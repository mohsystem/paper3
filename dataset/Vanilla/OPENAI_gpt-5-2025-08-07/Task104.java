public class Task104 {
    public static String handleInput(String input, int capacity) {
        if (capacity <= 0 || input == null) {
            return "";
        }
        int n = Math.min(input.length(), capacity);
        char[] buffer = new char[n];
        for (int i = 0; i < n; i++) {
            buffer[i] = input.charAt(i);
        }
        return new String(buffer);
    }

    public static void main(String[] args) {
        String[] inputs = {
            "Hello World",
            "",
            "Short",
            "ExactSize",
            "BoundaryTest"
        };
        int[] capacities = {5, 10, 10, 9, 0};
        for (int i = 0; i < inputs.length; i++) {
            String out = handleInput(inputs[i], capacities[i]);
            System.out.println("Case " + (i + 1) + ": [" + out + "]");
        }
    }
}