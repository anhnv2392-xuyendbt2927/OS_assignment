#include "pintos_thread.h"

// Forward declaration. This function is implemented in reaction-runner.c,
// but you needn't care what it does. Just be sure it's called when
// appropriate within reaction_o()/reaction_h().
void make_water();

struct reaction {
	int h_num;

	struct lock lock;
	struct condition done;
	struct condition h_new;
};

void
reaction_init(struct reaction *reaction)
{
	reaction->h_num = 0;

	lock_init(&reaction->lock);
	cond_init(&reaction->done);
	cond_init(&reaction->h_new);
}

void
reaction_h(struct reaction *reaction)
{
	lock_acquire(&reaction->lock);
	reaction->h_num++;
	
	cond_signal(&reaction->h_new, &reaction->lock);
	
	cond_wait(&reaction->done, &reaction->lock);

	lock_release(&reaction->lock);
}

void
reaction_o(struct reaction *reaction)
{
	lock_acquire(&reaction->lock);
	while (reaction->h_num < 2){
		cond_wait(&reaction->h_new, &reaction->lock);
	}
	make_water();
	reaction->h_num -= 2;
	
	cond_signal(&reaction->done, &reaction->lock);
	cond_signal(&reaction->done, &reaction->lock);

	lock_release(&reaction->lock);
}
