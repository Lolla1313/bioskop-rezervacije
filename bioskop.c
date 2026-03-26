#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> //za spavanje

sem_t semafor;

//definisemo strukturu za 1 sediste u bioskopu
struct Sediste {
    int broj;
    char kupac[40];
    int status;
    struct Sediste *sledece; //pokazivac na sl el u listi
};

//globalni pokazivac na pocetak liste
struct Sediste *glava = NULL;

//funk za ucitavanje iz datoteke i kreiranje liste
void ucitaj_iz_datoteke () {
    FILE *datoteka = fopen("sala.txt", "r"); //otvaranje fajla za citanje
     if (datoteka == NULL) {
        printf("\n Greška! Datoteka sala.txt. ne postoji!\n");
        return;
     }
     

     int privremeni_br, stat;
     char ime[40];
     while (fscanf(datoteka, "%d %d %s", &privremeni_br, &stat, ime) != EOF) { //cita br po br dok ne dodje do kraja e0f
        //novi cvor liste
        struct Sediste *novo = (struct Sediste*)malloc(sizeof(struct Sediste));
        novo->broj = privremeni_br;
        novo->status = stat;
        strcpy(novo->kupac, ime); //inicijalno nema kupca
        novo->status = 0; //sediste slobodno
        novo->sledece = NULL; //nis ne pokazuje jer je nov

        //povezivanje u listu
        if (glava == NULL) {
            glava = novo; //ako je lista prazna ovo je prvi element
        } else {
            struct Sediste *tekuci = glava;
            while (tekuci->sledece != NULL) {
                tekuci = tekuci->sledece; //idemo do posl elementa
            }
            tekuci->sledece = novo; //poslednji sad pokazuje na novo sediste
        }
     }
     fclose(datoteka);
     printf("\nSala je uspešno učitana u memoriju.\n");
}

void prikazi_plan_sedista() {
    struct Sediste *tekuci = glava; //krece od pocetka liste

    if (tekuci == NULL) {
        printf("\nGreska: Sala nije ucitana!");
        return;
    }
    printf("\n PLAN SEDISTA \n");
    while (tekuci != NULL) {
        printf("Sediste br. %d [%s]\n", tekuci->broj, tekuci->kupac);
        tekuci = tekuci->sledece; //pomeri se na sledecu "kutiju" u listi
    }
    printf("--------------------\n");
}

void snimi_u_datoteku () {
    sem_wait(&semafor); //zakljucaj, svi stoje dok ja pisem u fajl
    FILE *datoteka = fopen("sala.txt", "w"); //w itvara fajl ispocetka

if (datoteka == NULL ) {
    printf("\nGreska pri otvaranju datoteke za snimanje.\n");
    return;
}

struct Sediste *tekuci = glava;
while (tekuci != NULL) {
    fprintf(datoteka, "%d %d %s\n", tekuci->broj, tekuci->status, tekuci->kupac);
    tekuci = tekuci->sledece;
}
fclose(datoteka);
printf("\nIzvestaj uspesno snimljen u sala.txt!\n");
sem_post(&semafor); //otkljucaj, gotovo je mozemo poceti s rezervacijama
}

void* simulacija_kupca(void* arg) {
    int ID = *(int*)arg; //zvezdica pretvara obicnu funkciju u nesto sto niti razumeju
    printf("\n[Nit %d] Stigao na salter i cekam slobodan termin...\n", ID);

    sem_wait(&semafor);
    printf("[Nit %d] Uspešno rezervisao pristup! Radim nešto važno 3 sekunde...\n", ID);
    sleep(3); //simuliramo da proces traje 3 sek

    sem_post(&semafor);
    printf("[Nit %d] Zavrsio i oslobadjam salter\n", ID);
    return NULL;
} 

void glavni_meni () {
    int izbor;
    //iteracija
    do {
        printf("-- GLAVNI MENI - Sistem za rezervaciju karata --");
        printf("\nIzaberite opciju po zelji:\n");
        printf("1.Ucitaj plan sale iz datoteke\n");
        printf("2.Prikaži plan sedišta:\n");
        printf("3.Rezerviši sedište:\n");
        printf("4.Snimi izveštaj u datoteku\n");
        printf("5.Simulacija istovremenih rezervacija(Niti)\n");
        printf("0.Izlaz\n");
        printf("Vaš izbor: ");
        scanf("%d", &izbor);
    
    //selekcija, uslovno grananje
    switch(izbor) {  case 1: 
        ucitaj_iz_datoteke(); //pozivamo funkciju na citanje fajla
        break;
        case 2:
        prikazi_plan_sedista();
        //pozivamo funk za prikaz liste
        break;
        case 3: {
       if (glava == NULL) {
        printf("\n Greska: prvo ucitajte salu iz fajla(opcija 1)!\n");
        break;
       }
       int uspesno_rezervisano = 0;
       while (uspesno_rezervisano == 0) {
        int trazeno_sediste;
        printf("\nUkupno ima 5 sedista, izaberite broj sedista po zelji:");
       scanf("%d", &trazeno_sediste);
       if (trazeno_sediste == 0) break;

       sem_wait(&semafor);
       struct Sediste *tekuci = glava;
       int pronadjeno = 0;

       while (tekuci != NULL) {
        if (tekuci ->broj == trazeno_sediste) {
            pronadjeno = 1;
            if (tekuci->status == 1) {
                printf( "\n Sediste %d je vec rezervisano za %s\n", tekuci -> broj, tekuci -> kupac);
                
            } else { 
                printf ("Sediste je slobodno, unesite Vase ime: ");
                scanf ("%s", tekuci->kupac);
                tekuci -> status = 1; //menjamo slobodno u zauzeto ( 0 u 1)
            printf("\n--- REZERVACIJA USPESNA! Sediste %d je sada na ime: %s ---\n", tekuci->broj, tekuci->kupac);
                uspesno_rezervisano = 1;
        } 
            break;
        }
        tekuci = tekuci->sledece; // Traži dalje u listi
    }
    sem_post(&semafor);
        if (!pronadjeno) {
            printf("Sediste pod brojem %d ne postoji u ovoj sali.", trazeno_sediste);
        }
    }
        break;
} 
        case 4: 
        snimi_u_datoteku();
        //za upis u fajl
        break;
        case 0: printf("\nHvala na koriscenju!\n"); 
        break;
          case 5:
           pthread_t kupac1, kupac2;
          int ID1 =1, ID2 = 2;
          
          printf("Pokrecem dve niti istovremeno...\n");
          pthread_create(&kupac1, NULL, simulacija_kupca, &ID1);
          pthread_create(&kupac2, NULL, simulacija_kupca, &ID2);
          pthread_join(kupac1, NULL);
          pthread_join(kupac2, NULL);
            break;
          default:
        printf("Greska");
        break;
    } //kraj switch

} while (izbor != 0);  
} 
      

int main() {
    sem_init(&semafor, 0, 1);
    glavni_meni();
    sem_destroy(&semafor);
}
