#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <string>
#include <regex>


using namespace std;
#include "clutils.h"

int main(int argc, char **argv) {
   
   const char *clu_File = SRC_PATH "base.cl";  // path to OpenCL kernel file
  cluInit();

   cl::Program *program = cluLoadProgram(clu_File);
   cl::Kernel *kernel = cluLoadKernel(program, "network_sort");
    
    int a[] = {1, 4, 7, 13, 20};
    // structure qui contient les couples
    struct couple{
        int indice1;
        int indice2;
    };
   
    int size = sizeof(a) / sizeof(a[0]);

   cl::Buffer a_buffer(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
   clu_Queue->enqueueWriteBuffer(a_buffer, true, 0, size * sizeof(int), a);



    std::ifstream fichier("/Users/Diane/3A-sle/opencl/td3_1/size.txt");
    if (!fichier) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier." << std::endl;
        return 1;
    }

    std::string ligne;
  

    // Parcourir chaque ligne du fichier
    int ligne_cible = size; // Modifier pour le numéro de ligne souhaité (commence à 1)
 
    int compteur_lignes = 0;

    int compteur_blocs = 0;

    // Avancer jusqu'à la ligne cible
    while (std::getline(fichier, ligne) && ++compteur_lignes < ligne_cible) {
        // Ignorer les lignes précédentes
    }

    if (compteur_lignes == ligne_cible) {
        // Expressions régulières pour les blocs et les paires
        std::regex regex_bloc(R"(\[(.*?)\])");
        std::regex regex_paires(R"(\((\d+),(\d+)\))");

        auto blocs_begin = std::sregex_iterator(ligne.begin(), ligne.end(), regex_bloc);
        auto blocs_end = std::sregex_iterator();

      

        // Parcourir chaque bloc []
        for (std::sregex_iterator i = blocs_begin; i != blocs_end; ++i) {
            std::smatch bloc_match = *i;
            std::string contenu_bloc = bloc_match[1].str(); // Contenu entre []

            // Trouver toutes les paires (x, y) dans le bloc
            auto paires_begin = std::sregex_iterator(contenu_bloc.begin(), contenu_bloc.end(), regex_paires);
            auto paires_end = std::sregex_iterator();

            int compteur_paires = 0;
            std::vector<std::pair<int, int>> paires;

            for (std::sregex_iterator j = paires_begin; j != paires_end; ++j) {
                std::smatch paire_match = *j;
                int x = std::stoi(paire_match[1].str());
                int y = std::stoi(paire_match[2].str());
                paires.emplace_back(x, y);
                compteur_paires++;
            }

            // Affichage des résultats pour chaque bloc
            compteur_blocs++;
            clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(compteur_paires), cl::NullRange);
            int j = 0;
            // declarer des trucs comme "int a[n_elements]" peut causer des problemes si n_elements n'est pas constant
            struct couple *b = new struct couple[compteur_paires];
            
           
            std::cout << "Bloc " << compteur_blocs << " : [" << contenu_bloc << "]" << std::endl;
            std::cout << "Nombre de paires (x, y) : " << compteur_paires << std::endl;
            std::cout << "Paires : ";
            for (const auto& p : paires) {
                std::cout << "(" << p.first << "," << p.second << ") ";
                // t'avais écrit b[i] ici mais je suis presque sure que tu voulais dire b[j]
                b[j].indice1 = p.first;
                b[j].indice2 = p.second;
                j++;
            }
            std::cout << std::endl;
            
            kernel->setArg(0, a_buffer);      
            // c'est important de dire que tu veux interpreter le tableau de couples comme un tableau de entiers
            kernel->setArg(1, (int*) b);

            clu_Queue->finish();
            delete[] b;
        }


        // Affichage final pour la ligne
        std::cout << "Ligne " << ligne_cible << " : Nombre total de blocs : " << compteur_blocs << std::endl;
    } else {
        std::cerr << "Erreur : la ligne cible n'existe pas dans le fichier." << std::endl;
    }
  
    fichier.close();
    clu_Queue->enqueueReadBuffer(a_buffer, true, 0, size * sizeof(int), a);

    std::cout << "A :" << std::endl;
       for (int i = 0; i < size; i++) {
            std::cout << a[i] << " ";
        }
        std::cout << std::endl;
   

   // for(){
    //    clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(N), cl::NullRange);
   //     couple *b = new couple[N];

   //     kernel->setArg(0, a_buffer);      
   //     kernel->setArg(1, c_buffer);  
   //     kernel->setArg(2, size); 
  //  }
   // clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(N), cl::NullRange);
  //  clu_Queue->finish();

   // clu_Queue->enqueueReadBuffer(c_buffer, true, 0, N * sizeof(int), c);

     // Afficher le résultat de l'étape
   // std::cout << "C :" << std::endl;
   //     for (int i = 0; i < N; i++) {
   //         std::cout << c[i] << " ";
  //      }
    //    std::cout << std::endl;
   
   // delete[] c;
    return 0;

}
