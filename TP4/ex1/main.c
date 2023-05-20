#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define CIRCLE_SLICE 100

struct Color
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

typedef struct
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

Balle balle;

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

void draw_circle(Balle balle)
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
    glClear(GL_COLOR_BUFFER_BIT); // efface le buffer
    draw_circle(balle);
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

void *thread_balle(void *args)
{
    int index = 0;
    while (index < balle.nbRebonds + 1)
    {
        balle.x += balle.dx * balle.v;
        balle.y += balle.dy * balle.v;

        if (balle.x + balle.r / 2 >= WINDOW_WIDTH - balle.r / 2 || balle.x - balle.r / 2 <= balle.r / 2)
        {
            balle.dx *= -1;
            index++;
        }
        if (balle.y + balle.r / 2 >= WINDOW_HEIGHT - balle.r / 2 || balle.y - balle.r / 2 <= balle.r / 2)
        {
            balle.dy *= -1;
            index++;
        }

        // printf("r : %f | x : %f | y : %f | dx : %f | dy %f\n", balle.r, balle.x, balle.y, balle.dx, balle.dy);

        usleep(10000);
    }
    balle.vivante = 0;
    pthread_exit(EXIT_SUCCESS);
}

int main()
{
    pthread_t t[2];
    void *retval;
    int i;

    srand(time(NULL));

    balle.r = randomFloat(5, 30);
    balle.x = randomFloat(0.1 + balle.r * (3 / 2), WINDOW_WIDTH - balle.r * (3 / 2) - 0.1);
    balle.y = randomFloat(0.1 + balle.r * (3 / 2), WINDOW_HEIGHT - balle.r * (3 / 2) - 0.1);
    double a = randomFloat(0, 2 * M_PI);
    balle.dx = cos(a);
    balle.dy = sin(a);
    balle.v = randomFloat(3, 6);
    balle.couleur.r = randomFloat(0, 1);
    balle.couleur.g = randomFloat(0, 1);
    balle.couleur.b = randomFloat(0, 1);
    balle.nbRebonds = randomInt(1, 20);
    balle.vivante = 1;
    // printf("x : %f | y : %f | dx : %f | dy : %f\nr : %f | v : %f | nbRebonds : %i\n", balle.x, balle.y, balle.dx, balle.dy, balle.r, balle.v, balle.nbRebonds);

    pthread_create(&t[0], NULL, &thread_affichage, NULL);
    pthread_create(&t[1], NULL, &thread_balle, NULL);

    for (i = 0; i < 2; i++)
    {
        pthread_join(t[i], &retval);
        printf("Thread %i : return %i", i, *(int *)retval);
    }

    return EXIT_SUCCESS;
}