#include "pintos_thread.h"

struct station {
    int free_seats;             // number of free seats in the train, -if one exists at station-.
    int passengers_waiting;     // number of passengers waiting at station
    int passengers_leaving;     // number of passengers who have found a free seat
    struct condition closed;	// all leaving passengers have been seated
    struct condition open;	// train arrived with free seats
    struct lock lock;
};

void
station_init(struct station *station)
{	
	/* initially, station is empty */
    station->passengers_leaving = 0;
    station->passengers_waiting = 0;
	/* initialize station mutex and condition variables */
    cond_init(&station->closed);
    cond_init(&station->open);
    lock_init(&station->lock);
}

void
station_load_train(struct station *station, int count)
{
	lock_acquire(&station->lock); 		// enter critical section
    station->free_seats = count;

    while ((station->free_seats > 0 && station->passengers_waiting > 0) || station->passengers_leaving > 0) {
        cond_broadcast(&station->open, &station->lock);		// wake up all passengers waiting for a free seat
        cond_wait(&station->closed, &station->lock);		//waiting for all passengers to get on board
    }

    station->free_seats = 0;
    lock_release(&station->lock);		// leave critical section
}

void
station_wait_for_train(struct station *station)
{
    lock_acquire(&station->lock); 	// enter critical section
    station->passengers_waiting += 1;

    while (station->free_seats == 0)
        cond_wait(&station->open, &station->lock);	// waiting for a train with free seats

    station->passengers_leaving += 1;
    station->free_seats -= 1;
    station->passengers_waiting -= 1;

    lock_release(&station->lock);	// leave critical section
}

void
station_on_board(struct station *station)
{
	lock_acquire(&station->lock); 	// enter critical section
    station->passengers_leaving -= 1;
	// all leaving passengers are on board, and no other passengers can get on the train
    if ((station->free_seats == 0 || station->passengers_waiting == 0) && station->passengers_leaving == 0)
        cond_signal(&station->closed, &station->lock);

    lock_release(&station->lock);	// leave critical section
}
