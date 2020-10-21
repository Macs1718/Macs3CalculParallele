#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "ProdMatMat.hpp"

namespace {
void prodSubBlocks(int iRowBlkA, int iColBlkB, int iColBlkA, int szBlock,
                   const Matrix& A, const Matrix& B, Matrix& C) {
    // Mémoire chache associative : addr_ram(&x) ------> addr_cache = addr_ram % taille_du_cache
    // Donc si on lit &x puis &x+1024 pour un cache de taille 1024 : addr_cache identiques
    // Pour une boucle en k, puis en i puis en j
    // Donc pour une matrice de taille 1024, lire C(i,j) puis C(i,j+1) => on stocke les deux
    // valeurs au même endroit en cache => donc quand on lit C(i,j+1) en ram et qu'on le stocke
    // en cache : on doit "virer" l'ancienne valeur => on écrit l'ancienne valeur en RAM,
    // puis on stocke la nouvelle valeur à la place de l'ancienne valeur
    // ===> une écriture en RAM (vider la case de la mémoire cache) + une lecture (stocker la valeur
    // dans la case de la mémoire cache).
    // Donc opération très lente car beaucoup d'accès en RAM.
    // Cette opération ici on la fait 2n² fois (n = 1024) dimension de la matrice
    //
    // Sinon on intervertit les boucles en j, puis en k, puis en i : dans la boucle la plus interne,
    // on a C(i,j) => lecture contigüe en mémoire => remplissage de la mémoire cache avec une case de
    // la mémoire cache différente à chaque itération
    //      A(i,k) => idem, lecture contigüe => remplissage mémoire cache ok
    //      B(k,j) => ne varie pas dans la boucle la plus interne :
    // Par de reécriture en ram pour virer des valeurs en mémoire cache, ou alors un minimum (car dépassement
    // de la taille de la mémoire cache) => accès mémoire optimal pour ce type de boucle
    for (int k = iColBlkA; k < std::min(A.nbCols, iColBlkA + szBlock); k++)
      for (int j = iColBlkB; j < std::min(B.nbCols, iColBlkB + szBlock); j++)
        for (int i = iRowBlkA; i < std::min(A.nbRows, iRowBlkA + szBlock); ++i)
            C(i, j) += A(i, k) * B(k, j); // A(i,k) = A[i+k*nbRows]
}
const int szBlock = 256;
}  // namespace

Matrix operator*(const Matrix& A, const Matrix& B) {
  // Produit par bloc : on décompose A,B et C en blocs de taille szBlock : A_IJ, B_IJ, K_IJ
  // Et le produit matrice-matrice bloc par bloc :
  //     C_IJ = Sum_(k) A_IK.B_KJ (A_IK.B_KJ est un produit bloc matriciel-bloc matriciel)
  Matrix C(A.nbRows, B.nbCols, 0.0);
  for (int K = 0; K < A.nbCols; K += szBlock )
#   pragma omp parallel for
    for ( int I = 0; I < A.nbRows; I += szBlock )
      for ( int J = 0; J < B.nbCols; J += szBlock )
	   prodSubBlocks(I, J, K, szBlock, A, B, C);
  //prodSubBlocks(0, 0, 0, std::max({A.nbRows, B.nbCols, A.nbCols}), A, B, C);
  return C;
}
