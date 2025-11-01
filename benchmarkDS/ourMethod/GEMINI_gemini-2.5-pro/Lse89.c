// To compile: gcc -o app your_file_name.c -lmicrohttpd
// Requires libmicrohttpd development library to be installed.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <microhttpd.h>

#define PORT 5000
#define MAX_RESPONSE_SIZE 256

typedef struct {
    const char* code;
    double price;
} Product;

static Product product_prices[] = {
    {"PROD001", 19.99},
    {"PROD002", 25.50},
    {"PROD003", 5.75},
    {"PROD004", 99.99},
    {NULL, 0.0} // Sentinel
};

/**
 * @brief Returns the price for a given product code.
 *
 * @param product_code The code of the product.
 * @return double The price if found, otherwise -1.0.
 */
double get_product_price(const char* product_code) {
    if (product_code == NULL) {
        return -1.0;
    }
    for (int i = 0; product_prices[i].code != NULL; ++i) {
        if (strcmp(product_prices[i].code, product_code) == 0) {
            return product_prices[i].price;
        }
    }
    return -1.0;
}

static enum MHD_Result
send_json_response(struct MHD_Connection *connection, unsigned int status_code, const char *json_string) {
    struct MHD_Response *response;
    enum MHD_Result ret;

    response = MHD_create_response_from_buffer(strlen(json_string), (void *)json_string, MHD_RESPMEM_MUST_COPY);
    if (!response) {
        return MHD_NO;
    }

    MHD_add_response_header(response, "Content-Type", "application/json; charset=UTF-8");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}


static enum MHD_Result
answer_to_connection(void *cls, struct MHD_Connection *connection,
                     const char *url, const char *method,
                     const char *version, const char *upload_data,
                     size_t *upload_data_size, void **con_cls) {
    if (strcmp(method, "GET") != 0) {
        return send_json_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "{\"error\":\"Method Not Allowed\"}");
    }

    if (strcmp(url, "/price") != 0) {
        return send_json_response(connection, MHD_HTTP_NOT_FOUND, "{\"error\":\"Not Found\"}");
    }

    const char *code_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "code");
    const char *quantity_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "quantity");

    if (code_str == NULL || quantity_str == NULL) {
        return send_json_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Missing 'code' or 'quantity' parameter\"}");
    }

    double price = get_product_price(code_str);
    if (price < 0.0) {
        return send_json_response(connection, MHD_HTTP_NOT_FOUND, "{\"error\":\"Product code not found\"}");
    }

    char *endptr;
    errno = 0;
    long quantity = strtol(quantity_str, &endptr, 10);
    
    if (errno != 0 || *endptr != '\0' || endptr == quantity_str) {
        return send_json_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Quantity must be a valid integer\"}");
    }
    
    if (quantity <= 0 || quantity > 10000) {
        return send_json_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Invalid quantity. Must be between 1 and 10000.\"}");
    }

    double total_price = price * quantity;
    char response_buffer[MAX_RESPONSE_SIZE];
    snprintf(response_buffer, sizeof(response_buffer),
             "{\"product_code\":\"%s\", \"quantity\":%ld, \"unit_price\":%.2f, \"total_price\":%.2f}",
             code_str, quantity, price, total_price);

    return send_json_response(connection, MHD_HTTP_OK, response_buffer);
}

int main(void) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server daemon.\n");
        return 1;
    }
    
    printf("Server running on port %d\n", PORT);
    printf("\n--- Test Cases ---\n");
    printf("Run these commands in another terminal:\n");
    printf("1. Valid product and quantity: curl \"http://localhost:5000/price?code=PROD001&quantity=10\"\n");
    printf("2. Another valid product: curl \"http://localhost:5000/price?code=PROD004&quantity=2\"\n");
    printf("3. Invalid product code: curl \"http://localhost:5000/price?code=INVALID&quantity=5\"\n");
    printf("4. Invalid quantity (non-numeric): curl \"http://localhost:5000/price?code=PROD002&quantity=abc\"\n");
    printf("5. Missing quantity parameter: curl \"http://localhost:5000/price?code=PROD003\"\n");
    printf("--------------------\n\n");
    
    printf("Press Enter to stop the server.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    printf("Server stopped.\n");
    return 0;
}