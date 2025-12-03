/** 
 * @file interface.c
 * @brief Fonction d'affichage de menu et de navigation
*/

#include "filter.h"
#include "interface.h"
#include "lecture.h"
#include "utils.h"

// Dessiner une ligne horizontale non utilisé pour l'instant
void draw_line(WINDOW *win, int y, int x, int width) 
{
    if (width <= 0) return;
    char buf[1024];

    if (width >= (int)sizeof(buf))
        width = sizeof(buf) - 1;

    memset(buf, '-', width);
    buf[width] = '\0';

    mvwprintw(win, y, x, "%s", buf);
}

// Afficher la priorité avec une couleur spécifique
void print_priority( WINDOW *win, int y, int x, const char *label, const char *value )
{
    wattron( win, A_BOLD );
    mvwprintw( win, y, x, "%s", label );
    wattroff( win, A_BOLD );

    int prio = atoi( value );
    if ( prio >= 0 && prio <= 7 )
    {
        switch ( prio )
        {
            case 0:
            case 1:
            case 2:
                wattron( win, COLOR_PAIR(4) ); // Rouge pour Erreurs critiques
                break;
            case 3:
            case 4:
                wattron( win, COLOR_PAIR(5) ); // Jaune pour Warnings
                break;
            case 5:
            case 6:
            case 7:
                wattron( win, COLOR_PAIR(6) ); // Vert pour Info
                break;
        }
    }

    mvwprintw( win, y, x + strlen( label ), "%s", value );

    if ( prio >= 0 && prio <= 7 )
    {
        wattroff( win, COLOR_PAIR(4) );
        wattroff( win, COLOR_PAIR(5) );
        wattroff( win, COLOR_PAIR(6) );
    }
}

// Affichage d'une entrée du journal dans la fenêtre right
int affiche_entre( WINDOW *win , sd_journal *journal, int num )
{
    werase( win );
    box( win, 0, 0 );

    int h, w;
    getmaxyx( win, h, w );

    int y = 1;
    y++;
    // Titre
    {
        wattron( win, A_BOLD );
        mvwprintw( win, y, ( w - 9 ) / 2, "Entrée n°%d", num );
        wattroff( win, A_BOLD );
    }
    y += 2;
    mvwhline(win, y, 1, ACS_HLINE, w - 2);

    // Récupération de l'entrée courante
    Entry entre;
    if ( get_entry( journal, &entre, "" ) < 0 ) {
        y += 2;
        mvwprintw( win, y, 2, "Erreur : impossible de récupérer l'entrée." );
        wrefresh( win );
        return -1;
    }

    // PID / UID / GID sur la même ligne, colonnes égales
    y += 2;
    int col_w = ( w - 4 ) / 3; // 2 marges + 3 colonnes
    int col1x = 2;
    int col2x = col1x + col_w;
    int col3x = col2x + col_w;

    wattron( win, A_BOLD );
    mvwprintw( win, y, col1x, "PID :" );
    mvwprintw( win, y, col2x, "UID :" );
    mvwprintw( win, y, col3x, "GID :" );
    wattroff( win, A_BOLD );

    // valeurs ( impression limitée pour éviter débordement )
    mvwprintw( win, y, col1x + 6, "%.*s", col_w - 6, entre.pid );
    mvwprintw( win, y, col2x + 6, "%.*s", col_w - 6, entre.uid );
    mvwprintw( win, y, col3x + 6, "%.*s", col_w - 6, entre.gid );

    y += 2;
    mvwhline(win, y, 1, ACS_HLINE, w - 2);

    // Timestamp / Boot / Hostname ( chaque champ sur sa ligne, avec wrapping si utile )
    y += 2;
    wattron( win, A_BOLD );
    mvwprintw( win, y, 2, "TIMESTAMP :" );
    wattroff( win, A_BOLD );
    mvwprintw( win, y, 14, "%.*s", w - 16, entre.timestamp );

    y++;
    wattron( win, A_BOLD );
    mvwprintw( win, y, 2, "BOOT_ID :" );
    wattroff( win, A_BOLD );
    mvwprintw( win, y, 12, "%.*s", w - 14, entre.boot_id );

    y++;
    wattron( win, A_BOLD );
    mvwprintw( win, y, 2, "HOSTNAME :" );
    wattroff( win, A_BOLD );
    mvwprintw( win, y, 13, "%.*s", w - 15, entre.host );

    y += 2;
    mvwhline(win, y, 1, ACS_HLINE, w - 2);

    // Service / Priority
    y += 2;
    wattron( win, A_BOLD );
    mvwprintw( win, y, 2, "SERVICE :" );
    wattroff( win, A_BOLD );
    mvwprintw( win, y, 12, "%.*s", w - 14, entre.service );

    y++;
    print_priority( win, y, 2, "PRIORITY : ", entre.priority );

    y += 2;
    mvwhline(win, y, 1, ACS_HLINE, w - 2);

    // Message ( avec wrapping )
    {

        y += 2;
        wattron( win, A_BOLD );
        mvwprintw( win, y, 2, "Message :" );
        wattroff( win, A_BOLD );
        char *msg_ptr = entre.message;
        do
        {
            y++;
            mvwprintw( win, y, 2, "%.*s", w - 3, msg_ptr );
            msg_ptr += w - 3;
        } while ( msg_ptr <= entre.message + strlen( entre.message ) && *msg_ptr != '\0' && y < h - 2 );
    }

    // Fin
    wrefresh(win);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// CONSTANTE UTILE ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* ---------------------- CATEGORIES ---------------------- */
const char *categories[MAX_CATS] =
{
    "1- SD_JOURNAL_LOCAL_ONLY",
    "2- SD_JOURNAL_SYSTEM",
    "3- SD_JOURNAL_CURRENT_USER",
    "4- SD_JOURNAL_RUNTIME",
    "5- ALL_JOURNALS",
    "6- Aide",
    "7- Quitter"
};

/* -------------------- SOUS-MENUS -------------------------- */
const char *options[MAX_OPTS] = 
{
    "1-Filtrer par mots clés",
    "2-Filtrer par niveau de gravité",
    "3-Filtrer par service/programme",
    "4-Filtrer par date",
    "5-Aide",
    "6-Retour au menu principal",
};

/* ------------------- BANNIERE / LOGO ------------------------ */
const char *banniere_bienvenue[] =
{
"________________________________________________________________________________",
"",
"                               ✦✦   BIENVENUE   ✦✦",
"                             Dans notre LOG ANALYZER",
"",
"────────────────────────────────────────────────────────────────────────────────",
"",
"  Cet outil interactif vous permet d'explorer, filtrer et analyser vos logs",
"  système de manière claire, rapide et précise.",
"",
"  > Filtres disponibles :",
"        • Par mots-clés",
"        • Par niveau de gravité",
"        • Par service / programme",
"        • Par date",
"",
"  >Navigation :",
"        • ↑ / ↓  : Se déplacer dans les menus",
"        • ← / →  : Changer de catégorie / sous-menu",
"        • Entrée : Sélectionner",
"        • q      : Quitter ou revenir en arrière",
"",
"────────────────────────────────────────────────────────────────────────────────",
"",
"                          ✦ Analysez. Comprenez. Maîtrisez. ✦",
"",
"________________________________________________________________________________",
NULL
};

const char* JFT[]=
{
    "===By Jude , Fenohery , Tojo (JFT Tech Groupe) Version 1.0 ===",NULL
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// FONCTION D'AFFICHAGE DES MENU DES FILTRES ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* ------------------- FILTRES --------------------- */
void afficher_menu_mot_cle_left(WINDOW *win, char *buffer, int bufsize)
{
    werase(win);
    box(win, 0, 0);

    int y = 1, x = 2;
    mvwprintw(win, y++, x, "Filtrer par mots cles");
    mvwprintw(win, y++, x, "-----------------------");
    mvwprintw(win, y++, x, "Entrez le mot cle :");
    mvwprintw(win, y++, x, "(ex: error, sshd, kernel, warning...)");
    y++;
    mvwprintw(win, y++, x, "Mot cle courant :");

    int h, w;
    getmaxyx(win, h, w);
    (void)w;

    mvwprintw(win, h - 3, x, "Votre saisie : ");
    wrefresh(win);

    echo();
    curs_set(1);
    mvwgetnstr(win, h - 3, x + 15, buffer, bufsize - 1);
    noecho();
    curs_set(0);

    mvwprintw(win, h - 2, x, "Appuyez sur une touche pour continuer...");
    wrefresh(win);

}

int afficher_menu_gravite_left(WINDOW *win)
{
    const char* gravites[] =
    {
        "=> EMERGENCY ( 0 )",
        "=> ALERT     ( 1 )",
        "=> CRITICAL  ( 2 )",
        "=> ERROR     ( 3 )",
        "=> WARNING   ( 4 )",
        "=> NOTICE    ( 5 )",
        "=> INFO      ( 6 )",
        "=> DEBUG     ( 7 )",
        NULL
    };
    int choix = 0;
    int nb = 8;
    int ch;
    keypad(win, TRUE);

    while(1)
    {
        werase(win);
        box(win, 0, 0);

        int y = 1, x = 2;
        mvwprintw(win, y++, x, "Filtrer par niveau de gravite");
        mvwprintw(win, y++, x, "------------------------------");
        for (int i = 0; i < nb; i++)
        {
            if (i == choix) 
            {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, y++, x, "%s", gravites[i]);
            if (i == choix)
            {
                wattroff(win, A_REVERSE);
            }
        }
        int h , w ;
        getmaxyx(win, h, w);
        (void)h;
        mvwprintw(win, y + 1, x, "Utilisez les flèches ↑/↓ pour naviguer et Entrée pour sélectionner , 'q' pour quitter.");
        wrefresh(win);
        (void)w;
        ch = wgetch(win);
        switch (ch)
        {
            case KEY_UP:
                if (choix > 0) 
                {
                    choix--;
                }
                break;
            case KEY_DOWN:
                if (choix < nb - 1)
                {   
                    choix++;
                }
                break;
            case 10: // Entrée
                return choix; // 0..7
            case 'q':
                return -1; // annuler
        }
    }
}

void filtrer_par_service(WINDOW *win ,char *buffer , int bufsize) 
{
    werase(win);
    box(win, 0, 0);

    int y = 1, x = 2;
    mvwprintw(win, y++, x, "Filtrer par service/programme");
    mvwprintw(win, y++, x, "-----------------------");
    mvwprintw(win, y++, x, "Entrez le nom du service : ");
    mvwprintw(win, y++, x, "(ex: cron.service,init.scope,...)");
    y++;
    mvwprintw(win, y++, x, "Service/Programme: ");

    int h, w;
    getmaxyx(win, h, w);
    (void)w;    

    mvwprintw(win, h - 3, x, "Votre saisie : ");
    wrefresh(win);

    echo();
    curs_set(1);
    mvwgetnstr(win, h - 3, x + 15, buffer, bufsize - 1);
    noecho();
    curs_set(0);

    wrefresh(win);
}

void afficher_menu_date_left(WINDOW *win, char *date_debut, char *date_fin, int size)
{
    setlocale(LC_ALL, ""); 
    werase(win);
    box(win, 0, 0);

    int y = 1;
    int x = 2;

    mvwprintw(win, y++, x, "Filtrer par date");
    mvwprintw(win, y++, x, "--------------------");
    mvwprintw(win, y++, x, "Format accepte : AAAA-MM-JJ");
    mvwprintw(win, y++, x, "(Ex : 2025-11-10)");
    y++;

    // Saisie date début
    mvwprintw(win, y++, x, "Date de debut :");

    int h, w;
    getmaxyx(win, h, w);
    (void)h;
    (void)w;
    echo();
    curs_set(1);

    mvwgetnstr(win, y - 1, x + 17, date_debut, size - 1);
    y += 1;

    // Saisie date fin
    mvwprintw(win, y++, x, "Date de fin   :");
    mvwgetnstr(win, y - 1, x + 17, date_fin, size - 1);

    noecho();
    curs_set(0);

    wrefresh(win);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// FONCTION D'AFFICHAGE D'AIDE PRINCIPAL ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void afficher_page_aide()
{
    clear();
    curs_set(0);

    const char *aide[] =
    {
        "==============================================================",
        "                         PAGE D'AIDE",
        "==============================================================",
        "",
        "Bienvenue dans Log Analyzer !",
        "Cet outil permet d’analyser vos fichiers journaux (logs)",
        "de manière rapide, organisée et interactive.",
        "",
        "--------------------------------------------------------------",
        "1. NAVIGATION",
        "--------------------------------------------------------------",
        "- Utilisez ↑ et ↓ pour naviguer.",
        "- Entrée pour valider.",
        "- 'q' pour revenir.",
        "",
        "--------------------------------------------------------------",
        "2. FONCTIONS PRINCIPALES",
        "--------------------------------------------------------------",
        "A) Filtrer les logs",
        "   - Mots clés",
        "   - Gravité",
        "   - Service/Programme",
        "   - Date",
        "",
        "B) Explorer les logs",
        "   - Ligne par ligne",
        "   - Sauter les erreurs",
        "",
        "C) Résultats",
        "   - Nombre de lignes",
        "   - Exportation",
        "",
        "--------------------------------------------------------------",
        "3. RACCOURCIS",
        "--------------------------------------------------------------",
        "- ↑ / ↓ : navigation",
        "- q : retour",
        "- F10 : quitter l’application",
        "",
        "==============================================================",
        "",
        NULL
    };

    int total = 0;// compter le nombre de lignes d'aide
    while (aide[total] != NULL)
    {
        total++;
    }
    int start = 0;
    int rows, cols;
    (void)cols;
    int ch;

    keypad(stdscr, TRUE);// activer les touches spéciales

    while (1)
    {
        getmaxyx(stdscr, rows, cols);
        clear();

        int max_affiche = rows - 2;

        for (int i = 0; i < max_affiche && start + i < total; i++)// afficher les lignes d'aide
        {
            mvprintw(i + 1, 2, "%s", aide[start + i]);// marge de 2
        }    
        mvprintw(rows - 1, 2, "↑/↓ défiler | q pour revenir");// instructions

        refresh();

        ch = getch();

        if (ch == 'q')
        {
            break;
        }
        else if (ch == KEY_UP && start > 0)
        {
            // faire défiler vers le haut
            start--;
        }
        else if (ch == KEY_DOWN && start < total - max_affiche)// faire défiler vers le bas
        {
            start++;
        }
    }

    clear();
    refresh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// FONCTION D'AFFICHAGE DES SUB-MENU /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Afficher les filtres actifs
void afficher_filtres_actifs(WINDOW *win, FiltresActifs *filtres)
{
    int h, w;
    getmaxyx(win, h, w);
    (void)w;    

    int y = h - 6; // placer 6 lignes avant le bas
    int x = 2;

    mvwhline(win, y - 2, 1, ACS_HLINE, w - 2);
    mvwhline(win, y - 1, 1, ACS_HLINE, w - 2);
    mvwaddch(win, y - 2, 0, ACS_LLCORNER);
    mvwaddch(win, y - 2, w - 1, ACS_LRCORNER);
    mvwaddch(win, y - 1, 0, ACS_ULCORNER);
    mvwaddch(win, y - 1, w - 1, ACS_URCORNER);
    // Titre
    {
        wattron( win, A_BOLD );
        mvwprintw( win, y - 1, x, "[ Filtres actifs ]" );
        wattroff( win, A_BOLD );
    }
    x = 4;
    y++;
    // Mot clé
    if (strlen(filtres->mot_cle) > 0)
    {
        mvwprintw(win, y++, x, "[Mot clé] : ");
        wattron(win, COLOR_PAIR(6));
        wprintw(win, "%s", filtres->mot_cle);
        wattroff(win, COLOR_PAIR(6));
    }
    else
    {
        mvwprintw(win, y++, x, "[Mot clé] : Aucun");
    }

    // Gravité
    mvwprintw(win, y, x, "[Gravité] : ");
    if ( filtres->gravite < 0 )
    {
        mvwprintw(win, y, x + 12, "Aucune");
    }
    else
    {
        wattron(win, COLOR_PAIR(6));
        switch(filtres->gravite)
        {
            case 0: mvwprintw(win, y, x + 12, "EMERGENCY ( 0 )"); break;
            case 1: mvwprintw(win, y, x + 12, "ALERT     ( 1 )"); break;
            case 2: mvwprintw(win, y, x + 12, "CRITICAL  ( 2 )"); break;
            case 3: mvwprintw(win, y, x + 12, "ERROR     ( 3 )"); break;
            case 4: mvwprintw(win, y, x + 12, "WARNING   ( 4 )"); break;
            case 5: mvwprintw(win, y, x + 12, "NOTICE    ( 5 )"); break;
            case 6: mvwprintw(win, y, x + 12, "INFO      ( 6 )"); break;
            case 7: mvwprintw(win, y, x + 12, "DEBUG     ( 7 )"); break;
        }
        wattroff(win, COLOR_PAIR(6));
    }
    y++;

    // Service
    if (strlen(filtres->service) > 0)
    {
        mvwprintw(win, y++, x, "[Service] : ");
        wattron(win, COLOR_PAIR(6));
        mvwprintw(win, y - 1, x + 12, "%s", filtres->service);
        wattroff(win, COLOR_PAIR(6));
    }
    else
    {
        mvwprintw(win, y++, x, "[Service] : Aucun");
    }

    // Date
    if (strlen(filtres->date_debut) > 0 && strlen(filtres->date_fin) > 0)
    {
        mvwprintw(win, y++, x, "[Date] : ");
        wattron(win, COLOR_PAIR(6));
        mvwprintw(win, y - 1, x + 9, "De %s à %s", filtres->date_debut, filtres->date_fin);
        wattroff(win, COLOR_PAIR(6));
    }
    else
    {
        mvwprintw(win, y++, x, "[Date] : Aucune");
    }
}

void afficher_sous_menus(WINDOW *left, int cur_opt)
{
    mvwprintw(left, 0, 2, "[ Sous-menus ]");

    for (int i = 0; i < MAX_OPTS; i++)
    {
        if (i == cur_opt) 
        {
            wattron(left, COLOR_PAIR(1));
        }
        mvwprintw(left, 2 + i, 2, i == cur_opt ? "> %s" : "  %s",options[i]);

        if (i == cur_opt) 
        {
            wattroff(left, COLOR_PAIR(1));
        }
    }
}

// Afficher les détails complets d'une entrée dans une nouvelle fenêtre
void affiche_entry_details(sd_journal *journal, int entry_num)
{
    int h, w;
    getmaxyx(stdscr, h, w);

    // Fenêtre contenant le cadre
    WINDOW *win = newwin(h - 4, w - 3, 1, 1);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " Entrée %d ", entry_num);
    wrefresh(win);

    // PAD pour le contenu (taille généreuse)
    int pad_height = 2000;   // assez grand pour la plupart des entrées
    int pad_width  = w - 4;
    WINDOW *pad = newpad(pad_height, pad_width);

    // Footer
    creer_fenetre_footer( "Utilisez les flèches ↑/↓ pour défiler, 'q' pour quitter." );

    // Réinitialiser l’itérateur pour l’entrée courante
    sd_journal_restart_data(journal);

    const void *data;
    size_t data_size;
    int y = 0;

    // Énumérer les champs réels avec wrapping
    while (sd_journal_enumerate_available_data(journal, &data, &data_size) > 0)
    {
        const char *ptr = (const char *)data;
        int remaining = (int)data_size;

        while (remaining > 0 && y < pad_height - 1)
        {
            int chunk = pad_width - 2;
            if (chunk > remaining)
                chunk = remaining;

            mvwprintw(pad, y++, 0, "%.*s", chunk, ptr);

            ptr += chunk;
            remaining -= chunk;
        }
    }


    // SCROLLING
    int pos = 0;               // première ligne visible
    int visible_h = h - 4;     // hauteur visible interne
    int ch;

    prefresh(pad, pos, 0, 2, 2, h - 5, w - 3);

    keypad(win, TRUE);

    while ((ch = getch()) != 'q')
    {
        switch (ch)
        {
            case KEY_UP:
                if (pos > 0) pos--;
                break;
            case KEY_DOWN:
                if (pos < y - visible_h) pos++;
                break;
            case KEY_NPAGE:   // Page Down
                pos += visible_h;
                if (pos > y - visible_h) pos = y - visible_h;
                break;
            case KEY_PPAGE:   // Page Up
                pos -= visible_h;
                if (pos < 0) pos = 0;
                break;
        }

        if (pos < 0) pos = 0;
        if (pos > y - visible_h) pos = y - visible_h;
        if (pos < 0) pos = 0;

        prefresh(pad, pos, 0, 2, 2, h - 5, w - 3);
    }

    delwin(pad);
    delwin(win);

    // Nettoyer l'écran principal
    werase(stdscr);
    wrefresh(stdscr);
}

// Affichage du sous-menu
void sub_menu(WINDOW *left, WINDOW *right, sd_journal *journal)
{   
    int cur_opt = 0; // option courante
    int ch, r, cur_entry = 1;
    keypad(left, TRUE); // activer les touches spéciales

    // Initialisation des filtres actifs
    FiltresActifs filtres_actifs;
    memset(&filtres_actifs, 0, sizeof(filtres_actifs));
    filtres_actifs.gravite = -1; // Aucun niveau de gravité par défaut
    
    while (1)
    {
        werase(left);
        box(left, 0, 0);

        // Affichage des sous-menus et des filtres actifs à la fenêtre de gauche
        afficher_sous_menus(left, cur_opt);
        afficher_filtres_actifs(left, &filtres_actifs);
        
        // Affichage de l'entrée courante à la fenêtre de droite
        affiche_entre(right, journal, cur_entry);
        
        // Affichage du footer
        creer_fenetre_footer( "[ Navigation ] : ↑/↓ ←/→, [ Détails ] : 'd', [ Validé ] : 'Entrée', [ Retour ] : 'Échap'" );
        
        wrefresh(left);
        wrefresh(right);

        ch = wgetch(left);
        if (ch == 27) // Touche Échap
        {
            // Retour au menu principal
            break;
        }
        else if (ch == KEY_UP || ch == KEY_DOWN)
        {
            // Naviguer dans les options
            gerer_navigation_sub_menu(ch, &cur_opt);
        }
        else if (ch == KEY_LEFT || ch == KEY_RIGHT)
        {
            // Naviguer dans le journal
            r = gerer_navigation_journal(journal, ch, &cur_entry );
            if (r < 0)
            {
                // Erreur lors de la navigation*
                werase(right);
                box(right, 0, 0);
                mvwprintw(right, 1, 2, "Erreur lors de la navigation dans le journal.");
                wrefresh(right);
            }

            applique_filtres_personnalises( journal, &filtres_actifs, ch == KEY_RIGHT ? 1 : 0 );
        }
        else if ( ch == 'd' || ch == 'D' )
        {
            // Afficher les détails complets de l'entrée courante
            affiche_entry_details( journal, cur_entry );
        }
        else if (ch == 10) // Entrée
        {
            int quitter = entree_sub_menu(left, cur_opt, &filtres_actifs);
            if (quitter)
            {
                // Retour au menu principal
                break;
            }

            if ( cur_opt == 1 || cur_opt == 2 )
            {
                // Appliquer les filtres via API systemd
                applique_filtres_systemd( journal, &filtres_actifs );
                cur_entry = 1; // Revenir à la première entrée
            }
        }

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// FONCTION DE GESTION D'ACTION //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Gestion de la navigation dans le menu principal
void gerer_navigation_menu_principal(int ch, int *cur_cat)
{
    switch (ch)
    {
        case KEY_UP:
            if (*cur_cat > 0) 
            {
                (*cur_cat)--;
            }
        break;

        case KEY_DOWN:
            if (*cur_cat < MAX_CATS - 1) 
            {
                (*cur_cat)++;
            }
        break;
    }
}

// Gestion de la navigation dans les sous-menus
void gerer_navigation_sub_menu(int ch, int *cur_opt)
{
    switch (ch)
    {
        case KEY_UP:
            if (*cur_opt > 0) 
            {
                (*cur_opt)--;
            }
        break;

        case KEY_DOWN:
            if (*cur_opt < MAX_OPTS - 1) 
            {
                (*cur_opt)++;
            }
        break;
    }
}

// Gestion de la navigation dans le journal
// Retourne 1 si passage à l'entrée précédente ou suivante réussi, 0 si le début ou la fin du journal est atteint
// Retourne < 0 en cas d'erreur
int gerer_navigation_journal(sd_journal *journal, int ch, int *cur_entry)
{
    int r = 0;

    switch (ch)
    {
        case KEY_LEFT:
            r = sd_journal_previous(journal);
            if (r < 0)
            {
                fprintf(stderr, "Erreur lors de la navigation vers l'entrée précédente\n");
                return r;
            }

            if (r > 0 && *cur_entry > 1)
            {
                (*cur_entry)--;
                return 1;
            }
            // r == 0 -> début du journal
            return 0;
        case KEY_RIGHT:
            r = sd_journal_next(journal);
            if (r < 0)
            {
                fprintf(stderr, "Erreur lors de la navigation vers l'entrée suivante\n");
                return r;
            }

            if (r > 0)
            {
                (*cur_entry)++;
                return 1;
            }
            // r == 0 -> fin du journal
            return 0;
    }
    return 0;
} 

// Gestion des entrées dans le menu principal
// Retourne 1 pour quitter l'application, 0 pour continuer
int entree_menu_principal(WINDOW *left, WINDOW *right,int cur_cat)
{
    // Cas où aide et quitter sont sélectionnés dans le menu principal
    switch (cur_cat)
    {
    case 5: // Aide
        afficher_page_aide();
        return 0;
        break;

    case 6: // Quitter
        return 1;
        break;
    
    default:
        // Ouverture du journal en fonction de la catégorie sélectionnée et affichage du sous-menu
        open_log( left, right, cur_cat + 1 ); // ouvrir le journal correspondant à la catégorie sélectionnée
        break;
    }
    
    return 0;
}

// Gestion des entrées dans les sous-menus
// Retourne 1 pour quitter le sous-menu, 0 pour continuer
int entree_sub_menu(WINDOW *left, int opt, FiltresActifs *filtre_actifs)
{
    switch (opt)
    {
    case 0:
        // Filtrer par mots clés
        afficher_menu_mot_cle_left(left, filtre_actifs->mot_cle, 128);
        break;

    case 1:
        // Filtrer par niveau de gravité
        int ret = afficher_menu_gravite_left(left);
        if (ret >= 0 && ret <= 7)
        {
            filtre_actifs->gravite = ret;
        }
        break;

    case 2:
        // Filtrer par service/programme
        filtrer_par_service(left, filtre_actifs->service, 128);
        break;

    case 3:
        // Filtrer par date
        afficher_menu_date_left(left, filtre_actifs->date_debut, filtre_actifs->date_fin, 20);
        break;

    case 4:
        // Afficher la page d'aide
        afficher_page_aide(); // afficher_page_aide_sub_menu() à implémenter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        break;

    default:
        // Retour au menu principal
        return 1;
        break;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// FONCTION D'INITIALISATION ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initialiser_ncurses()
{
    setlocale(LC_ALL, "");
    initscr();
    set_escdelay(25);
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors())
    {
        start_color();
        use_default_colors(); // permettre arrière-plan transparent

        init_pair(1, COLOR_BLACK,  COLOR_CYAN);     // Sélection menu
        init_pair(2, COLOR_CYAN,   -1);             // Titres
        init_pair(3, COLOR_MAGENTA,-1);             // Sous-titres
        init_pair(4, COLOR_RED,    -1);             // Erreurs
        init_pair(5, COLOR_YELLOW, -1);             // Warnings
        init_pair(6, COLOR_GREEN,  -1);             // OK / Infos
        init_pair(7, COLOR_WHITE,  -1);             // Texte normal
        init_pair(8, COLOR_BLACK,  COLOR_WHITE);    // Entrée utilisateur
        init_pair(9, COLOR_BLUE,   -1);             // Lignes & séparateurs
    }
}

void creer_fenetres(WINDOW **left, WINDOW **right)
{
    int h, w;
    getmaxyx(stdscr, h, w);
    
    int left_w = (w * 30) / 100; // 30%
    int right_w = (w - left_w) - 4;
    int win_h ;
    if (h - 2 > 3)
    {
        win_h = h - 2;
    }
    else
    {
        win_h = 3;
    }

    if (left_w < 20)
    {
        left_w = 20;
    }
    if (right_w < 20)
    { 
        right_w = 20;
    }
    
    *left  = newwin(win_h - 2, left_w, 1, 1);
    *right = newwin(win_h - 2, right_w, 1, left_w + 2);
    wrefresh(stdscr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// FONCTION D'AFFICHAGE DU MENU PRINCIPAL ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void dessiner_logo(WINDOW *win) 
{
    werase(win); 
    box(win, 0, 0); 

    int h, w;
    getmaxyx(win, h, w); // obtenir dimensions de la fenêtre

    int y = 1, x = 2; // marge haute et gauche

    // Affichage du texte principal
    for (int i = 0; banniere_bienvenue[i] != NULL; i++)
    {
        if (y >= h - 2) 
        {
            break;
        }            // ne pas dépasser la hauteur
        int len = strlen(banniere_bienvenue[i]);
        if (len > w - 4) 
        {
            len = w - 4;
        }    // s'assurer de ne pas dépasser la largeur
        mvwprintw(win, y++, x, "%.*s", len, banniere_bienvenue[i]);
    }

    // Affichage de la signature JFT en bas à droite
    int sig_y = h - 2 - 0; // ligne avant le bas (0 lignes marge)
    for(int i = 0; JFT[i] != NULL; i++)
    {
        int len = strlen(JFT[i]);
        if (len > w - 4) 
        {
            len = w - 4; 
        } // ne pas dépasser largeur
        int sig_x = w - len - 2;       // marge de 2 à droite
        if (sig_x < 2) 
        {
            sig_x = 2; 
        }     // s'assurer que sig_x ne dépasse pas à gauche
        mvwprintw(win, sig_y + i, sig_x, "%.*s", len, JFT[i]);
    }

    wrefresh(win); 
}

// Affiche les catégories dans le menu de gauche
void afficher_categories(WINDOW *left, int cur_cat)
{
    werase(left);
    box(left, 0, 0);

    mvwprintw(left, 0, 2, "[ Catégories ]");

    for (int i = 0; i < MAX_CATS; i++)
    {
        if (i == cur_cat) 
        {
            wattron(left, COLOR_PAIR(1));
        }
        mvwprintw(left, 2 + i, 2, i == cur_cat ? "> %s" : "  %s", categories[i]);

        if (i == cur_cat) 
        {
            wattroff(left, COLOR_PAIR(1));
        }
    }
    wrefresh(left);
}

void creer_fenetre_footer( const char *msg )
{
    int h, w;
    getmaxyx(stdscr, h, w);
    WINDOW *footer = newwin(3, w - 3, h - 3, 1);
    werase(footer);
    box(footer, 0, 0);

    mvwprintw(footer, 1, ( w - strlen(msg) ) / 2, "%.*s", w - 2, msg);
    wrefresh(footer);
}

int afficherMenu()
{
    initialiser_ncurses();

    WINDOW *left, *right;
    creer_fenetres(&left, &right);

    if ( !left || !right )
    {
        endwin();
        fprintf(stderr, "Erreur lors de la création des fenêtres ncurses.\n");
        return -1;
    }

    int cur_cat = 0;

    int ch;
    while(1)
    {
        afficher_categories(left, cur_cat);
        dessiner_logo(right);
        creer_fenetre_footer( "Utilisez les flèches ↑/↓ pour naviguer, Entrée pour sélectionner, 'q' pour quitter." );

        ch = getch();
        if (ch == 'q') 
        {
            break;
        }
        else if (ch == 10)
        {
            int quitter = entree_menu_principal(left,right,cur_cat);

            if(quitter)
            {
                break ;
            }
        }
        else
        {
            gerer_navigation_menu_principal(ch, &cur_cat);
        }
    }

    endwin();
    return 0;
}