#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define CIRCLE_SLICE 20

sem_t sem_secure_thread_init;
sem_t sem_balle;
sem_t sem_plein;

const int nbBalles = 10;

struct Color
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

typedef struct Balle
{
    double x;
    double y;
    double dx;
    double dy;
    double r;
    struct Color couleur;
    double v;
    int nbRebonds;
    int vivante;
} Balle;

Balle *balle;
pthread_t *tballe;

double randomFloat(float min, float max)
{
    double r = (double)rand() / (double)RAND_MAX;
    return (min + r * (max - min));
}

int randomInt(int min, int max)
{
    double r;
    do
    {
        r = rand() % (max + 1);
    } while (r < min);
    return (int)r;
}

void drawCircle(Balle balle)
{
    if (balle.vivante == 1)
    {
        int i;
        glColor3f(balle.couleur.r, balle.couleur.g, balle.couleur.b);
        glBegin(GL_POLYGON);
        for (i = 0; i < CIRCLE_SLICE; i++)
        {
            double angle = 2 * M_PI * i / CIRCLE_SLICE;
            glVertex2d(balle.x + cos(angle) * balle.r, balle.y + sin(angle) * balle.r);
        }
        glEnd();
    }
}

void affiche()
{
    glutPostRedisplay();
    usleep(10000);
}

void display()
{
    int i;
    glClear(GL_COLOR_BUFFER_BIT); // efface le buffer
    for (i = 0; i < nbBalles; i++)
        drawCircle(balle[i]);
    glutSwapBuffers(); // affiche à l'écran le buffer dans lequel nous avons dessiné
}

void *thread_affichage(void *args)
{
    int nbArg = 1;
    glutInit(&nbArg, (char **)args);                       // fonction d'initialisation
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);           // utilise le codage RGB en double buffer
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);       // taille de la fenêtre
    glutInitWindowPosition(100, 100);                      // position de la fenêtre
    glutCreateWindow("TP4 prog-multi | Ex1");              // nom de la fenêtre
    glClearColor(0, 0, 0, 0);                              // défini la couleur d'effacement par défaut
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1.0, 1.0); // défini la position du repère dans la scène visible
    glutDisplayFunc(display);                              // défini la fonction appelée pour afficher la scène
    glutIdleFunc(affiche);                                 // défini la fonction lancée lorsque le système n'a plus rien à faire
    glutMainLoop();                                        // fonction bloquante permettant la gestion de l'affichage
    pthread_exit(EXIT_SUCCESS);
}

void *thread_balle(void *arg)
{
    int i = *((int *)arg);
    int index = 0;
    sem_post(&sem_secure_thread_init);
    // printf("Balle %i : thread cree.\n", i);
    while (index < balle[i].nbRebonds + 1)
    {
        // printf("Balle %i : loop.\n", i);
        balle[i].x += balle[i].dx * balle[i].v;
        balle[i].y += balle[i].dy * balle[i].v;

        if (balle[i].x + balle[i].r / 2 >= WINDOW_WIDTH - balle[i].r / 2 || balle[i].x - balle[i].r / 2 <= balle[i].r / 2)
        {
            balle[i].dx *= -1;
            index++;
        }
        if (balle[i].y + balle[i].r / 2 >= WINDOW_HEIGHT - balle[i].r / 2 || balle[i].y - balle[i].r / 2 <= balle[i].r / 2)
        {
            balle[i].dy *= -1;
            index++;
        }
        // printf("r : %f | x : %f | y : %f | dx : %f | dy %f\n", balle[i].r, balle[i].x, balle[i].y, balle[i].dx, balle[i].dy);
        usleep(10000);
    }
    balle[i].vivante = 0;
    sem_post(&sem_balle);
    pthread_exit(EXIT_SUCCESS);
}

void nouvelleBalle(int i)
{
    balle[i].r = randomFloat(5, 30);
    balle[i].x = randomFloat(0.1 + balle[i].r * (3 / 2), WINDOW_WIDTH - balle[i].r * (3 / 2) - 0.1);
    balle[i].y = randomFloat(0.1 + balle[i].r * (3 / 2), WINDOW_HEIGHT - balle[i].r * (3 / 2) - 0.1);
    double a = randomFloat(0, 2 * M_PI);
    balle[i].dx = cos(a);
    balle[i].dy = sin(a);
    balle[i].v = randomFloat(3, 10);
    balle[i].couleur.r = randomFloat(0, 1);
    balle[i].couleur.g = randomFloat(0, 1);
    balle[i].couleur.b = randomFloat(0, 1);
    balle[i].nbRebonds = randomInt(10, 20);
    balle[i].vivante = 1;
    // printf("%i ~ x : %f | y : %f | dx : %f | dy : %f | r : %f | v : %f | nbRebonds : %i\n", i, balle[i].x, balle[i].y, balle[i].dx, balle[i].dy, balle[i].r, balle[i].v, balle[i].nbRebonds);
    pthread_create(&tballe[i], NULL, &thread_balle, &i);
    sem_wait(&sem_secure_thread_init);
}

int main()
{
    pthread_t taffiche;
    int index;
    int i;

    tballe = calloc(nbBalles, sizeof(pthread_t));
    balle = calloc(nbBalles, sizeof(Balle));

    for (i  = 0; i < nbBalles; i++)
        balle[i].vivante = 0;

    sem_init(&sem_secure_thread_init, 0, 0);
    sem_init(&sem_balle, 0, nbBalles);
    sem_init(&sem_plein, 0, 0);

    srand(time(NULL));

    pthread_create(&taffiche, NULL, &thread_affichage, NULL);

    while (1)
    {
        sem_wait(&sem_balle);
        int random = randomInt(0, 1);
        printf("randomInt : %i\n", random);
        sleep(random);
        for (index = 0; index < nbBalles; index++)
            if (balle[index].vivante == 0)
                break;
        nouvelleBalle(index);
    }

    for (i = 0; i < nbBalles; i++)
        pthread_join(tballe[i], NULL);

    pthread_join(taffiche, NULL);

    return EXIT_SUCCESS;
}