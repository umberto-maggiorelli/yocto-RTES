#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

struct bandierine_t {
	int id_g1, id_g2;
	int id_portatore;
	int n_pronti;
	pthread_t t_g;
	pthread_t t_g1, t_g2;
	sem_t s_tutti_pronti;
	sem_t s_inizio_gioco;
	sem_t s_fine_gioco;
	sem_t s_bandierina;
	sem_t s_inseguimento;
	sem_t s_mutex;
} bandierine;

void attendi_giocatori(struct bandierine_t *b);
void via(struct bandierine_t *b);
int risultato_gioco(struct bandierine_t *b);
void attendi_il_via(struct bandierine_t *b, int id);
int bandierina_presa(struct bandierine_t *b, int id);
int sono_salvo(struct bandierine_t *b);
int ti_ho_preso(struct bandierine_t *b, int id);
void *giocatore(void *arg);
void *giudice();
void init_bandierine(struct bandierine_t *b);




void attendi_giocatori(struct bandierine_t *b){
        sem_wait(&b->s_tutti_pronti);
}

void via(struct bandierine_t *b){
        sem_post(&b->s_inizio_gioco);
        sem_post(&b->s_inizio_gioco);
        sem_wait(&b->s_fine_gioco);
}

int risultato_gioco(struct bandierine_t *b){
	return b->id_portatore;
}


void attendi_il_via(struct bandierine_t *b, int id){
	sem_wait(&b->s_mutex);
	b->n_pronti++;
	sem_post(&b->s_mutex);
	if (b->n_pronti >= 2)
		sem_post(&b->s_tutti_pronti);
	sem_wait(&b->s_inizio_gioco);
}

int bandierina_presa(struct bandierine_t *b, int id){
	if (sem_trywait(&b->s_bandierina) == 0){
		b->id_portatore = id;
		return 1;
	}
	return 0;
}

int sono_salvo(struct bandierine_t *b){
	return sem_trywait(&b->s_inseguimento) == 0 ? 1 : 0;
}

int ti_ho_preso(struct bandierine_t *b, int id){
	if (sem_trywait(&b->s_inseguimento) == 0){
		b->id_portatore = id;
		return 1;
	}
	return 0;
}



void *giocatore(void *arg){
	int id = *((int*)arg);
	int altro_id;
	if (id == 1)
		altro_id = 2;
	else
		altro_id = 1;
		
	printf("Giocatore %d: 'Sono pronto, attendo il via del giudice'\n", id);
	attendi_il_via(&bandierine, id);
	// corri e tenda di prendere la bandierina
	if (bandierina_presa(&bandierine, id)){
		printf("Giocatore %d: 'Ho preso la bandierina!'\n", id);
		usleep(1);
		if (sono_salvo(&bandierine))
			printf("Giocatore %d: 'Sono salvo in base!'\n", id);
	}
	else {
		usleep(1);
		if (ti_ho_preso(&bandierine, id))
			printf("Giocatore %d: 'Ho preso Giocatore %d!'\n", id, altro_id);
	}
	sem_wait(&bandierine.s_mutex);
	bandierine.n_pronti--;
	sem_post(&bandierine.s_mutex);
	if (bandierine.n_pronti <= 0)
		sem_post(&bandierine.s_fine_gioco);
}

void *giudice(){
	printf("Giudice: 'Attendo che i giocatori siano pronti'\n");
	attendi_giocatori(&bandierine);
	printf("Giudice 'Pronti, Partenza... VIA!'\n");
	via(&bandierine);
	printf("Giudice: 'Il vincitore è: %d'\n", risultato_gioco(&bandierine));
}

void init_bandierine(struct bandierine_t *b){
	b->id_g1 = 1;
	b->id_g2 = 2;
	b->id_portatore = 0;
	b->n_pronti = 0;

	sem_init(&b->s_inizio_gioco, 0, 0);
	sem_init(&b->s_fine_gioco, 0, 0);
	sem_init(&b->s_tutti_pronti, 0, 0);
	sem_init(&b->s_bandierina, 0, 1);
	sem_init(&b->s_inseguimento, 0 ,1);
	sem_init(&b->s_mutex, 0, 1);
	
	if (pthread_create(&b->t_g, NULL, giudice, NULL) != 0)
		printf("Errore nella creazione del thread giudice\n");
	if (pthread_create(&b->t_g1, NULL, giocatore, &b->id_g1) != 0)
		printf("Errore nella creazione del thread giocatore 1\n");
	if (pthread_create(&b->t_g2, NULL, giocatore, &b->id_g2)!= 0)
		printf("Errore nella creazione del thread giocatore 2\n");

	pthread_join(b->t_g, NULL);
	pthread_join(b->t_g1, NULL);
	pthread_join(b->t_g2, NULL);
}

int main(int argc, char **argv){
	init_bandierine(&bandierine);
	return 1;
}
