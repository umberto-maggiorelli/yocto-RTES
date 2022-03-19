#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define N_CORRIDORI 10

struct corsa_t {
	int id_corridore[N_CORRIDORI];
	// int id_arbitro;				// non necessario
	int id_primo;
        int id_ultimo;
        int n_corridori;				// cambio nome variabile da n_pronti a n_corridori
	pthread_t t_arbitro;
	pthread_t t_corridore[N_CORRIDORI];
	sem_t s_tutti_pronti;
	sem_t s_inizio_corsa;
	sem_t s_fine_corsa;
	sem_t s_mutex;
	// sem_t s_classifica;				// non necessario

} corsa;

void corridore_attendivia(struct corsa_t *corsa, int numerocorridore);
void corridore_arrivo(struct corsa_t *corsa, int numerocorridore);
void arbitro_attendicorridori(struct corsa_t *corsa);
void arbitro_via(struct corsa_t *corsa);
void arbitro_risultato(struct corsa_t *corsa, int *primo, int *ultimo);
void *corridore(void *arg);
void *arbitro(); 					// l'arbitro non ha bisogno di un id
void init_corsa(struct corsa_t *s);




void corridore_attendivia(struct corsa_t *corsa, int numerocorridore){
	sem_wait(&corsa->s_mutex); 
	corsa->n_corridori++;				// segno di essere arrivato sulla linea di partenza
	printf("Corridore %d: 'Sono pronto al via'\n", numerocorridore);
	if(corsa->n_corridori >= N_CORRIDORI)		// se sono l'ultimo arrivato sveglio l'arbitro
		sem_post(&corsa->s_tutti_pronti);
	sem_post(&corsa->s_mutex);
	sem_wait(&corsa->s_inizio_corsa);		// attendo che l'arbitro dia il via
}

// non è necessario il semaforo 's_classifica' per segnare l'ordine di arrivo al traguardo basta utilizzare il semaforo 's_mutex' che 
// serve anche per incrementare il contatore dei corridori arrivati per poi segnalare l'arbitro che annuncia il primo e l'ultimo arrivato
void corridore_arrivo(struct corsa_t *corsa, int numerocorridore){
	sem_wait(&corsa->s_mutex);
	printf("Corridore %d: 'Ho tagliato il traguardo'\n", numerocorridore);
	if(corsa->id_primo == 0)			// se sono il primo ad arrivare mi segno come primo
		corsa->id_primo = numerocorridore;
	else						// tutti quelli che non arrivano primi si segnano come ultimi
		corsa->id_ultimo = numerocorridore;
	
	corsa->n_corridori++;				// segno di avere tagliato il traguardo
	if(corsa->n_corridori >= N_CORRIDORI)		// se sono l'ultimo arrivato sveglio l'arbitro
		sem_post(&corsa->s_fine_corsa);
	sem_post(&corsa->s_mutex);
}


void arbitro_attendicorridori(struct corsa_t *corsa){
	printf("Arbitro: 'Attendo i corridori'\n");
	sem_wait(&corsa->s_tutti_pronti);		// aspetto che tutti i corridori siano pronti
}

void arbitro_via(struct corsa_t *corsa){
	printf("Arbitro: 'Pronti, Partenza, ...VIA!'\n");
	corsa->n_corridori = 0;				// risetto a zero la variabile che serve a risvegliarmi terminata la gara per annunciare il risultato
	for(int i = 0; i < N_CORRIDORI; i++)		// do il via a tutti i corridori
		sem_post(&corsa->s_inizio_corsa);
}

void arbitro_risultato(struct corsa_t *corsa, int *primo, int *ultimo){
	sem_wait(&corsa->s_fine_corsa);			// attendo l'arrivo di tutti i corridori per poi annunciare il risultato
	printf("Arbitro: 'Il primo classificato è: Corridore %d; l'ultimo classificato è: Corridore %d'\n", *primo, *ultimo);
}



void *corridore(void *arg){
	int numerocorridore = *((int *)arg); 		// id del corridore
	// vado sulla pista
	corridore_attendivia(&corsa, numerocorridore);
	// corro più veloce possibile
	corridore_arrivo(&corsa, numerocorridore);
	// torno a casa
}

void *arbitro(){
	// vado sulla pista
	arbitro_attendicorridori(&corsa);
	// pronti, attenti, ...
	arbitro_via(&corsa);
	// attendo che arrivino al termine
	arbitro_risultato(&corsa, &corsa.id_primo, &corsa.id_ultimo);
}

void init_corsa(struct corsa_t *s){
	for(int i = 1; i <= N_CORRIDORI; i++)
		s->id_corridore[i-1] = i;
	// s->id_arbitro = 0;				// non necessario
	
	s->id_primo = 0;
	s->id_ultimo = 0;

	s->n_corridori = 0;

	sem_init(&s->s_tutti_pronti, 0, 0);
	sem_init(&s->s_inizio_corsa, 0, 0);
	sem_init(&s->s_fine_corsa, 0, 0);
	sem_init(&s->s_mutex, 0, 1);
	// sem_init(&s->s_classifica, 0, 1);		// non necessario

	// creo il thread arbitro
	if(pthread_create(&s->t_arbitro, NULL, arbitro, NULL) != 0)
      		printf("Errore nella creazione del thread arbitro\n");
	// creo i thread corridori
	for(int c = 0; c < N_CORRIDORI; c++)
		if(pthread_create(&s->t_corridore[c], NULL, corridore, (void *) &s->id_corridore[c]) != 0)
			printf("Errore nella creazione del thread corridore: %d\n", s->id_corridore[c]);
	
	pthread_join(s->t_arbitro, NULL);
	for(int c = 0; c < N_CORRIDORI; c++)
		pthread_join(s->t_corridore[c], NULL);
}

int main(int argc, char **argv){
	init_corsa(&corsa);
	return 1;
}
