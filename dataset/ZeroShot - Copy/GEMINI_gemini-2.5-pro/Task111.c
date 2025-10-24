/*
 * This code requires the xmlrpc-c library.
 * On Debian/Ubuntu: sudo apt-get install libxmlrpc-c-dev
 * On RedHat/CentOS: sudo yum install xmlrpc-c-devel
 *
 * To compile:
 * gcc your_file_name.c -o server_c $(xmlrpc-c-config abyss-server --cflags --libs)
 *
 * Then run:
 * ./server_c
 *
 * Note: A 'main class' concept doesn't exist in C.
 * The code is structured within a main function as is standard for C.
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>

/*
 * Secure method handler for "sample.add"
 * It validates parameter count, types, and checks for integer overflow.
*/
static xmlrpc_value * 
sample_add(xmlrpc_env *   const env, 
           xmlrpc_value * const param_array, 
           void *         const user_data ATTR_UNUSED) {

    xmlrpc_int32 addend, adder;

    /* Secure: Decompose value and validate types/count in one step. */
    xmlrpc_decompose_value(env, param_array, "(ii)", &addend, &adder);
    if (env->fault_occurred) {
        return NULL; /* Error is already set in env */
    }

    /* Secure: Check for integer overflow before performing the operation */
    if ((adder > 0 && addend > INT_MAX - adder) || (adder < 0 && addend < INT_MIN - adder)) {
        xmlrpc_env_set_fault(env, XMLRPC_INTERNAL_ERROR, "Integer overflow detected.");
        return NULL;
    }

    /* Return the result */
    return xmlrpc_build_value(env, "i", addend + adder);
}

/*
 * Secure method handler for "sample.divide"
 * It validates types and checks for division by zero.
*/
static xmlrpc_value *
sample_divide(xmlrpc_env *   const env,
              xmlrpc_value * const param_array,
              void *         const user_data ATTR_UNUSED) {

    double dividend, divisor;
    xmlrpc_decompose_value(env, param_array, "(dd)", &dividend, &divisor);
    if (env->fault_occurred) {
        return NULL;
    }

    if (divisor == 0.0) {
        xmlrpc_env_set_fault(env, XMLRPC_INTERNAL_ERROR, "Division by zero is not permitted.");
        return NULL;
    }

    return xmlrpc_build_value(env, "d", dividend / divisor);
}


int main(int const argc, const char ** const argv) {

    xmlrpc_server_abyss_parms serverparm;
    xmlrpc_registry * registryP;
    xmlrpc_env env;
    int port = 8083;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    /* Initialize the XML-RPC C environment */
    xmlrpc_env_init(&env);

    /* Create a new method registry */
    registryP = xmlrpc_registry_new(&env);
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_registry_new() failed. %s\n", env.fault_string);
        exit(1);
    }

    /* Add our "add" method to the registry */
    xmlrpc_registry_add_method(
        &env,                  /* environment */
        registryP,             /* registry */
        NULL,                  /* method name default - taken from handler */
        "sample.add",          /* method name */
        &sample_add,           /* method handler function */
        NULL);                 /* user data */
    
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_registry_add_method() failed for add. %s\n", env.fault_string);
        exit(1);
    }
    
    /* Add our "divide" method to the registry */
    xmlrpc_registry_add_method(
        &env, &registryP, NULL, "sample.divide", &sample_divide, NULL);
    
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_registry_add_method() failed for divide. %s\n", env.fault_string);
        exit(1);
    }

    /* Set up the server parameters */
    serverparm.config_file_name = NULL; /* Use default Abyss config */
    serverparm.registryP        = registryP;
    serverparm.port_number      = port;
    /* For security, use a proper logging setup in a real application */
    serverparm.log_file_name    = "/tmp/xmlrpc_c_log";

    printf("Starting XML-RPC C server on port %d...\n", port);
    printf("Accepting requests. (Press Ctrl-C to stop)\n");
    
    printf("\n--- Test Cases (run from a separate client) ---\n");
    printf("You can use a Python client for testing:\n");
    printf("----------------------------------------------\n");
    printf("import xmlrpc.client\n");
    printf("s = xmlrpc.client.ServerProxy('http://localhost:8083/RPC2')\n");
    printf("# Test Case 1: Add\n");
    printf("print(s.sample.add(5, 3))\n");
    printf("# Test Case 2: Add negative numbers\n");
    printf("print(s.sample.add(-10, -20))\n");
    printf("# Test Case 3: Divide\n");
    printf("print(s.sample.divide(100.0, 8.0))\n");
    printf("# Test Case 4: Divide by zero (error handling)\n");
    printf("try:\n    print(s.sample.divide(5, 0))\nexcept Exception as e:\n    print(e)\n");
    printf("# Test Case 5: Incorrect parameter count (error handling)\n");
    printf("try:\n    print(s.sample.add(1, 2, 3))\nexcept Exception as e:\n    print(e)\n");
    printf("----------------------------------------------\n");

    /* Create, start, and run the server */
    xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));
    if (env.fault_occurred) {
        fprintf(stderr, "xmlrpc_server_abyss() failed. %s\n", env.fault_string);
        exit(1);
    }

    /* xmlrpc_server_abyss() never returns */

    return 0;
}