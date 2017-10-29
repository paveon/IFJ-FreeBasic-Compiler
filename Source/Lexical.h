#ifndef LEXICAL_H
#define LEXICAL_H

//TODO napsat rozsahlejsi komentare

/*
 * Primarni funkce pro lexikalni analyzu. Precte cely vstup na stdin a vytvori zretezeny seznam tokenu.
 *
 * @brief primarni funkce, vytvori seznam zretezenych tokenu, pracuje dokud neprijde EOF
 *
 * @return v pripade uspechu vrati 'true' a v pripade chyby vraci 'false'
 */
bool Lexical();


/* Funkce pro uklid pameti, pouziva se i pri kriticke chybe */
void LexCleanup();


#endif //LEXICAL_H
