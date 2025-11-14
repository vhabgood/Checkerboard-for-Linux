#include "checkers_types.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <stdio.h>
#include <errno.h>

#define MAX_INSTANCES 100

static sem_t *h_named_sem = SEM_FAILED;
static char sem_name_buffer[100];

/* Get the application instance by creating a named semamphore.  If the semaphore already
 * exists then that application instance is running.
 * The first instance number is 0, then 1, 2, ...
 * If any errors, the function return is non-zero and the application instance returned is 0.
 */
int get_app_instance(char *name, int *app_instance)
{
	int i;

	*app_instance = 0;
	for (i = 0; i < MAX_INSTANCES; ++i) {
		sprintf(sem_name_buffer, "/%s_sem%d", name, i);

		// Try to create the semaphore exclusively
		h_named_sem = sem_open(sem_name_buffer, O_CREAT | O_EXCL, 0644, 1);
		
		if (h_named_sem != SEM_FAILED) {
			// We successfully created a new semaphore, so this is our instance
			*app_instance = i;
			return 0;
		}

		if (errno != EEXIST) {
			// An error other than "it already exists" occurred
			return 1;
		}
		// If we are here, semaphore already exists, so we try the next one
	}

	/* Too many instances. */
	return 1;
}


void close_app_instance()
{
	if (h_named_sem != SEM_FAILED) {
		sem_close(h_named_sem);
		sem_unlink(sem_name_buffer);
	}
}