/*
NOTE: This code requires the 'xmlrpc-c' library. It cannot be compiled
as a single file without it.

On Debian/Ubuntu, install the library:
sudo apt-get install libxmlrpc-core-c3-dev

To compile:
gcc -o server Task111.c $(xmlrpc-c-config abyss-server --cflags --libs)

This program starts a server and waits for requests. There are no client
test cases in this file. You can use the provided Python client script below
to test the running server.

--- Python Test Client (save as client.py) ---
import xmlrpc.client

proxy = xmlrpc.client.ServerProxy("http://localhost:8080/RPC2")

# Test Case 1
r1 = proxy.sample.add(5, 3)
print(f"Test 1: sample.add(5, 3) = {r1}")

# Test Case 2
r2 = proxy.sample.add(-10, 5)
print(f"Test 2: sample.add(-10, 5) = {r2}")

# Test Case 3
r3 = proxy.sample.add(0, 0)
print(f"Test 3: sample.add(0, 0) = {r3}")

# Test Case 4
r4 = proxy.sample.add(12345, 67890)
print(f"Test 4: sample.add(12345, 67890) = {r4}")

# Test Case 5
r5 = proxy.sample.subtract(10, 4)
print(f"Test 5: sample.subtract(10, 4) = {r5}")
------------------------------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>

static xmlrpc_value* sample_add(xmlrpc_env*   const env,
                                xmlrpc_value* const param_array,
                                void*         const user_data) {
    xmlrpc_int32 x, y;

    /* Parse our argument array. */
    xmlrpc_decompose_value(env, param_array, "(ii)", &x, &y);
    if (env->fault_occurred)
        return NULL; /* Return error */

    /* Return our result. */
    return xmlrpc_build_value(env, "i", x + y);
}

static xmlrpc_value* sample_subtract(xmlrpc_env*   const env,
                                     xmlrpc_value* const param_array,
                                     void*         const user_data) {
    xmlrpc_int32 x, y;

    xmlrpc_decompose_value(env, param_array, "(ii)", &x, &y);
    if (env->fault_occurred)
        return NULL;

    return xmlrpc_build_value(env, "i", x - y);
}

int main(int const argc, const char ** const argv) {

    const int port = 8080;
    xmlrpc_server_abyss_parms serverparm;
    xmlrpc_registry * registryP;
    xmlrpc_env env;

    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    /* Create the method registry */
    registryP = xmlrpc_registry_new(&env);
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_registry_new() failed. %s\n", env.fault_string);
        exit(1);
    }
    
    /* Add our methods to the registry. */
    xmlrpc_registry_add_method(
        &env, registryP, NULL, "sample.add", &sample_add, NULL);
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_registry_add_method() failed. %s\n", env.fault_string);
        exit(1);
    }

    xmlrpc_registry_add_method(
        &env, registryP, NULL, "sample.subtract", &sample_subtract, NULL);
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_registry_add_method() failed. %s\n", env.fault_string);
        exit(1);
    }

    /* Set up the server */
    serverparm.config_file_name = NULL; /* Use default settings */
    serverparm.registryP        = registryP;
    serverparm.port_number      = port;
    serverparm.log_file_name    = "/dev/null"; /* Suppress logging */

    printf("XML-RPC server listening on port %d...\n", port);
    printf("Press Ctrl-C to terminate.\n");

    xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_server_abyss() failed. %s\n", env.fault_string);
        exit(1);
    }

    /* xmlrpc_server_abyss() never returns */
    
    /* Clean up (unreachable code) */
    xmlrpc_registry_free(registryP);
    xmlrpc_env_clean(&env);

    return 0;
}