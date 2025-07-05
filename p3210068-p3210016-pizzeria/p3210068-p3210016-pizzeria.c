#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "p3210068-p3210016-pizzeria.h"

pthread_mutex_t revenue_mutex;
pthread_mutex_t stats_mutex;
pthread_mutex_t screen_mutex;
pthread_mutex_t manufacturer_mutex;
pthread_mutex_t oven_mutex;
pthread_mutex_t packer_mutex;
pthread_mutex_t distributor_mutex;

pthread_cond_t manufacturer_cond;
pthread_cond_t oven_cond;
pthread_cond_t packer_cond;
pthread_cond_t distributor_cond;
pthread_cond_t order_delivered_cond;

int in_oven = 0;
int totalSales = 0;
int plainPizzas = 0;
int specialPizzas = 0;
int succesfull = 0;
int failed= 0;
int max_service = 0;
int max_cooling = 0;
int packer_available = 1;

int total_service=0;
int total_cooling=0;


void print_output(const char* message) {
    pthread_mutex_lock(&screen_mutex);
    printf("%s\n", message);
    pthread_mutex_unlock(&screen_mutex);
}

void register_order(int id) {
    pthread_mutex_lock(&stats_mutex);
    succesfull++;
    pthread_mutex_unlock(&stats_mutex);
    char message[100];
    sprintf(message, "The order with number %d has registered.", id);
    print_output(message);
}

void fail_order(int id) {
    pthread_mutex_lock(&stats_mutex);
    failed++;
    pthread_mutex_unlock(&stats_mutex);
    char message[100];
    sprintf(message, "The order with number %d has failed.", id);
    print_output(message);
}

void prepare_order(int id) {
    unsigned int prep_time = (unsigned int)Tprep;
  	max_service=max_service+prep_time;
    sleep(prep_time);
}

void bake_pizzas(int id, int num_pizzas) {
    unsigned int bake_time = (unsigned int)Tbake;
    sleep(bake_time);
    char message[100];
    sprintf(message, "Order number %d with %d pizzas was baked in %u minutes.", id, num_pizzas, bake_time);
    print_output(message);
}

void pack_order(int id, int num_pizzas) {
    unsigned int pack_time = (unsigned int)Tpack;
    max_service=max_service+pack_time;
    sleep(pack_time);
}

void deliver_order(int id,unsigned int delivery) {
    sleep(delivery);
    char message[100];
    sprintf(message, "Order number %d was delivered in %u minutes.", id, delivery);
    print_output(message);

    pthread_mutex_lock(&distributor_mutex);
    pthread_cond_signal(&order_delivered_cond);
    pthread_mutex_unlock(&distributor_mutex);
    
    
    int cooling_time=(int)delivery-Tbake;
    pthread_mutex_lock(&stats_mutex);
    total_cooling+=cooling_time;
    if (max_cooling<cooling_time){
        	max_cooling=cooling_time;
        }
    pthread_mutex_unlock(&stats_mutex);
}

void* customer_thread(void* arg) {
    int id = *((int*)arg);
    free(arg);

    unsigned int order_time = (unsigned int)(Torderlow + rand() % (Torderhigh - Torderlow + 1));
    sleep(order_time);

    int num_pizzas = Norderlow + rand() % (Norderhigh - Norderlow + 1);
    int is_plain = (rand() / (double)RAND_MAX) < Pplain ? 1 : 0;
    int total_price = is_plain ? num_pizzas * Cplain : num_pizzas * Cspecial;

    unsigned int payment_time = (unsigned int)(Tpaymentlow + rand() % (Tpaymenthigh - Tpaymentlow + 1));
    sleep(payment_time);

    int payment_fails = (rand() / (double)RAND_MAX) < Pfail ? 1 : 0;

    if (payment_fails) {
        fail_order(id);
    } else {
        register_order(id);

        pthread_mutex_lock(&revenue_mutex);
        totalSales += total_price;
        if (is_plain) {
            plainPizzas += num_pizzas;
        } else {
            specialPizzas += num_pizzas;
        }
        pthread_mutex_unlock(&revenue_mutex);

        pthread_mutex_lock(&manufacturer_mutex);
        pthread_cond_signal(&manufacturer_cond);
        pthread_mutex_unlock(&manufacturer_mutex);

        prepare_order(id);

        pthread_mutex_lock(&oven_mutex);
        while (in_oven >= Noven) {
            pthread_cond_wait(&oven_cond, &oven_mutex);
        }
        in_oven++;
        pthread_mutex_unlock(&oven_mutex);

        bake_pizzas(id, num_pizzas);


        pthread_mutex_lock(&oven_mutex);
        in_oven--;
        pthread_cond_broadcast(&oven_cond);
        pthread_mutex_unlock(&oven_mutex);


        pthread_mutex_lock(&packer_mutex);
        while (!packer_available) {
            pthread_cond_wait(&packer_cond, &packer_mutex);
        }
        packer_available = 0;
        pthread_mutex_unlock(&packer_mutex);

        pack_order(id, num_pizzas);
           
        unsigned int delivery_time = (unsigned int)(Tdellow + rand() % (Tdelhigh - Tdellow + 1));
        deliver_order(id,delivery_time);

        pthread_mutex_lock(&packer_mutex);
        packer_available = 1;
        pthread_cond_signal(&packer_cond);
        pthread_mutex_unlock(&packer_mutex);
        
        pthread_mutex_lock(&stats_mutex);
        int service_time=delivery_time+order_time+payment_time+Tbake+Tprep+Tpack;
        total_service+=service_time;
        if (max_service<service_time){
        	max_service=service_time;
        }
        pthread_mutex_unlock(&stats_mutex);
    }

    return NULL;
}

int main(int argc, char* argv[]) {

     if (argc < 3) {
        printf("Usage: ./program Ncust seed\n");
        return 1;
    }
    
    int Ncust = atoi(argv[1]);
    unsigned int seed = (unsigned int)atoi(argv[2]);
    
    srand(seed);
    
    pthread_mutex_init(&revenue_mutex, NULL);
    pthread_mutex_init(&stats_mutex, NULL);
    pthread_mutex_init(&screen_mutex, NULL);
    pthread_mutex_init(&manufacturer_mutex, NULL);
    pthread_mutex_init(&oven_mutex, NULL);
    pthread_mutex_init(&packer_mutex, NULL);
    pthread_mutex_init(&distributor_mutex, NULL);

    pthread_cond_init(&manufacturer_cond, NULL);
    pthread_cond_init(&oven_cond, NULL);
    pthread_cond_init(&packer_cond, NULL);
    pthread_cond_init(&distributor_cond, NULL);
    pthread_cond_init(&order_delivered_cond, NULL); 

    pthread_t* threads = (pthread_t*)malloc(Ncust * sizeof(pthread_t));

    for (int i = 0; i < Ncust; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads[i], NULL, customer_thread, id);
    }

    for (int i = 0; i < Ncust; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);

    pthread_mutex_destroy(&revenue_mutex);
    pthread_mutex_destroy(&stats_mutex);
    pthread_mutex_destroy(&screen_mutex);
    pthread_mutex_destroy(&manufacturer_mutex);
    pthread_mutex_destroy(&oven_mutex);
    pthread_mutex_destroy(&packer_mutex);
    pthread_mutex_destroy(&distributor_mutex);

    pthread_cond_destroy(&manufacturer_cond);
    pthread_cond_destroy(&oven_cond);
    pthread_cond_destroy(&packer_cond);
    pthread_cond_destroy(&distributor_cond);
    pthread_cond_destroy(&order_delivered_cond);

    printf("\nTotal sales: %d\n", totalSales);
    printf("Total plain pizzas: %d\n", plainPizzas);
    printf("Total special pizzas: %d\n", specialPizzas);
    printf("Max Service time: %d\n", max_service);
    printf("Max Cooling time: %d\n", max_cooling);
    double avg_service=(double)total_service/succesfull;
    double avg_cooling=(double)total_cooling/succesfull;
    printf("Average Service time: %.2f\n", avg_service);
    printf("Average Cooling time: %.2f\n", avg_cooling);

    return 0;
}

