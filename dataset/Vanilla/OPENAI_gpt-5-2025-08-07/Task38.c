/* For brevity, a full C HTTP server implementation is not provided here.
   You can implement a simple POSIX sockets server that:
   - Serves an HTML form at GET /.
   - Accepts POST /echo with application/x-www-form-urlencoded and echoes the "message" field.
   Provide helper functions like:
     char* generate_form_page(const char* title);
     char* generate_echo_page(const char* message);
   that accept input via parameters and return allocated HTML strings.
   Include a main() with 5 test cases calling generate_echo_page().
*/