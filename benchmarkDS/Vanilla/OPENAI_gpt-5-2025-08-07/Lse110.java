import java.io.IOException;

public class Lse110 {
    public static int runIdCommand(String username) {
        try {
            ProcessBuilder pb = new ProcessBuilder("id", username);
            pb.inheritIO();
            Process p = pb.start();
            return p.waitFor();
        } catch (IOException | InterruptedException e) {
            return 1;
        }
    }

    public static void main(String[] args) {
        if (args.length >= 1) {
            int status = runIdCommand(args[0]);
            System.out.println(status);
        } else {
            String[] tests = {"root", "daemon", "nobody", "bin", "this_user_does_not_exist_12345"};
            for (String u : tests) {
                int status = runIdCommand(u);
                System.out.println("Exit status for '" + u + "': " + status);
            }
        }
    }
}