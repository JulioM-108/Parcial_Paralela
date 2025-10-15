ls
./Ejecutables/filtro_pth /Images/damma.ppm Ouput_Images/damma_blur2.ppm --f blur
./Ejecutables/filtro_pth Images/damma.ppm Ouput_Images/damma_blur2.ppm --f blur
./Ejecutables/filtro_omp Images/sulfur.pgm Ouput_Images/sulfur_N1.pgm
g++ -fomp src/filterer_omp.cpp -o filtro.omp
g++ -fomep src/filterer_omp.cpp -o filtro.omp
g++ -fopenmp -o filtro_omp src/filterer_omp.cpp
clear
g++ -fopenmp -o filtro_omp src/filterer_omp.cpp
g++ -fopenmp -o filtro_omp src/filterer_omp.cpp
./Ejecutables/filtro_omp Images/sulfur.pgm Ouput_Images/sulfur_N1.pgm
exit
ls
exit
