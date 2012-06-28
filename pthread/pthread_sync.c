#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

struct foo {
	int f_count;
	pthread_mutex_t f_lock;
};

struct foo *foo_alloc(void)
{
	struct foo *fp;
	if((fp = malloc(sizeof(struct foo))) != 0) {
		fp->f_count = 0;
		if(pthread_mutex_init(&fp->f_lock, NULL) != 0) {
			free(fp);
			return NULL;
		}
	}
	return fp;
}

void foo_hold(struct foo *fp) {
	pthread_mutex_lock(&fp->f_lock);
	fp->f_count++;
	pthread_mutex_unlock(&fp->f_lock);
}

void foo_rele(struct foo *fp)
{
	pthread_mutex_lock(&fp->f_lock);
	if(--fp->f_count == 0) {
		pthread_mutex_unlock(&fp->f_lock);
		pthread_mutex_destroy(&fp->f_lock);
		free(fp);
	} else {
		pthread_mutex_unlock(&fp->f_lock);
	}
}

void foo_free(struct foo *fp)
{
	free(fp);
}

void *ptr_fn(void *arg)
{
	struct foo *fp = (struct foo *)arg;
	foo_hold(fp);
	pthread_t pid = pthread_self();
	printf("after foo_hold in thread %d, foo->f_count is %d.\n", 
		(unsigned int)pid, fp->f_count);
	pthread_exit((void *)0);
}

void main(void)
{
	int err, i;
	pthread_t tid;
	void *tret;

	printf("pthread mutex demo begins.\n");

	struct foo *fp = foo_alloc();
	if(!fp) {
		printf("no memory");
		exit(0);
	}

	printf("1. foo->f_count is %d.\n", fp->f_count);

	printf("2. foo->f_count increment begins.\n");

	for(i = 0; i < 100; i++) {
		err = pthread_create(&tid, NULL, ptr_fn, fp);
		if(err != 0) {
			printf("can't create thread");
		}
	}
	
	pthread_join(tid, &tret);

	printf("3. foo->f_count increment ends.\n");

	foo_free(fp);

	printf("4. after foo_rele, object foo has been freed.\n");

	exit(1);
}
