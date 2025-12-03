/**
 * @file main.c
 * @brief Point d'entrée du programme
 *
 * @details
 * Outil de diagnostic interactif des fichiers journaux (logs) sous Linux.
 *
 * @section interface Interface et navigation
 * - Menu textuel en console
 * - Navigation via clavier dans le fichier
 *
 * @section filtres Filtres et recherche avancée
 * - Filtrage par mots-clés, expressions régulières (motifs) et dates
 * - Classification par niveau de gravité (INFO, WARNING, ERROR)
 * - Regroupement par service/programme
 * - Combinaison de filtres
 * - Alertes personnalisées
 *
 * @section stats Statistiques et visualisation
 * - Calcul et affichage des statistiques de base (nombre de lignes, avertissements, etc.)
 * - Rapport d'erreurs : lister et compter les messages d'erreurs distincts
 *
 * @section autres Autres fonctionnalités
 * - Export des résultats : possibilité d’enregistrer la sortie filtrée dans un fichier texte
 * - Aide intégrée : menu d'aide résumant les touches et commandes disponibles
 *
 * @author Jude
 * @author Fenohery
 * @author Tojo
 *
 * @version 1.0
 * @date 2025-09-28
*/

#include "filter.h"
#include "interface.h"
#include "lecture.h"
#include "utils.h"

int main()
{
    int code = afficherMenu();

    if (code == 0)
    {
        printf("Sortie de l’application.\n");
    }
    else
    {
        printf("Sélection = %d\n", code);
    }

    return 0;
}