#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "BENSCHILLIBOWL.h"

// Feel free to play with these numbers! This is a great way to
// test your implementation.
#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread funtion that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long)tid;

    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        Order* order = malloc(sizeof(Order));
        if (!order) {
            perror("Failed to allocate memory for order");
            exit(EXIT_FAILURE);
        }

        order->menu_item = PickRandomMenuItem();
        order->customer_id = customer_id;
        order->next = NULL;

        AddOrder(bcb, order);
        printf("Customer #%d added Order #%d: %s\n", customer_id, order->order_number, order->menu_item);
    }

    return NULL;
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurants until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
  int cook_id = (int)(long)tid;
    int orders_fulfilled = 0;

    while (1) {
        // Get an order from the restaurant
        Order* order = GetOrder(bcb);

        // If no more orders are left, break out of the loop
        if (!order) {
            break;
        }

        // Fulfill the order
        printf("Cook #%d fulfilled Order #%d: %s\n", cook_id, order->order_number, order->menu_item);
        free(order); // Free the memory for the order
        orders_fulfilled++;
    }

    printf("Cook #%d fulfilled %d orders\n", cook_id, orders_fulfilled);
    return NULL;
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {
    srand(time(0)); // Seed the random number generator

    // Open the restaurant
    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);

    // Create customer and cook threads
    pthread_t customers[NUM_CUSTOMERS];
    pthread_t cooks[NUM_COOKS];

    for (long i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customers[i], NULL, BENSCHILLIBOWLCustomer, (void*)i);
    }

    for (long i = 0; i < NUM_COOKS; i++) {
        pthread_create(&cooks[i], NULL, BENSCHILLIBOWLCook, (void*)i);
    }

    // Wait for customers to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    // Wait for cooks to finish
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_join(cooks[i], NULL);
    }

    // Close the restaurant
    CloseRestaurant(bcb);
}