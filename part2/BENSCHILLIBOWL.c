#include "BENSCHILLIBOWL.h"
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    int i = rand() % BENSCHILLIBOWLMenuLength;
    return BENSCHILLIBOWLMenu[i];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL* bcb = malloc(sizeof(BENSCHILLIBOWL));
    if (!bcb) {
        perror("Failed to allocate memory for restaurant");
        exit(EXIT_FAILURE);
    }

    bcb->max_size = max_size;
    bcb->expected_num_orders = expected_num_orders;
    bcb->current_size = 0;
    bcb->orders_handled = 0;
    bcb->orders = NULL;
    bcb->next_order_number = 0;

    pthread_mutex_init(&(bcb->mutex), NULL);
    pthread_cond_init(&(bcb->can_add_orders), NULL);
    pthread_cond_init(&(bcb->can_get_orders), NULL);

    printf("Restaurant is open!\n");
    return bcb;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&(bcb->mutex));
    while (bcb->orders_handled < bcb->expected_num_orders) {
        pthread_cond_broadcast(&(bcb->can_get_orders));
        pthread_mutex_unlock(&(bcb->mutex));
        usleep(100);
        pthread_mutex_lock(&(bcb->mutex));
    }
    pthread_mutex_unlock(&(bcb->mutex));

    // Free remaining orders (if any)
    while (bcb->orders != NULL) {
        Order* order = bcb->orders;
        bcb->orders = bcb->orders->next;
        free(order);
    }

    pthread_mutex_destroy(&(bcb->mutex));
    pthread_cond_destroy(&(bcb->can_add_orders));
    pthread_cond_destroy(&(bcb->can_get_orders));

    free(bcb);
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&(bcb->mutex));

    while (IsFull(bcb)) {
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }

    order->order_number = ++(bcb->next_order_number); // Assign unique order number
    AddOrderToBack(&(bcb->orders), order);
    bcb->current_size++;

    pthread_cond_signal(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));

    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&(bcb->mutex));

    while (IsEmpty(bcb)) {
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }

    Order* order = bcb->orders;
    bcb->orders = bcb->orders->next;
    bcb->current_size--;
    bcb->orders_handled++;

    pthread_cond_signal(&(bcb->can_add_orders));
    pthread_mutex_unlock(&(bcb->mutex));

    return order;
}

// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
  return bcb->current_size == 0;
}

bool IsFull(BENSCHILLIBOWL* bcb) {
  return bcb->current_size >= bcb->max_size;
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) {
        *orders = order;
    } else {
        Order* current = *orders;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = order;
    }
    order->next = NULL;
}

