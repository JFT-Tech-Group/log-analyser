#ifndef INTERFACE_H
#define INTERFACE_H

#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CATS 7
#define MAX_OPTS 6

typedef struct FiltresActifs
{
    char mot_cle[128];
    int gravite;
    char service[128];
    char date_debut[20];
    char date_fin[20];
} FiltresActifs;

void initialiser_ncurses();
void creer_fenetres(WINDOW **left, WINDOW **right);
void creer_fenetre_footer( const char *msg );
int afficherMenu();
void affiche_entry_details(sd_journal *journal, int entry_num);
void sub_menu(WINDOW *left, WINDOW *right, sd_journal *journal);
int gerer_navigation_journal(sd_journal *journal, int ch, int *cur_entry);
void afficher_sous_menus(WINDOW *left, int cur_opt);
int entree_sub_menu(WINDOW *left, int opt, FiltresActifs *filtre_actifs);
void gerer_navigation_menu_principal(int ch, int *cur_cat);
int entree_menu_principal(WINDOW *left, WINDOW *right,int cur_cat);
void gerer_navigation_sub_menu(int ch, int *cur_opt);

#endif