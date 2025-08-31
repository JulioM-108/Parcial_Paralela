ls
ls
ls
g++ proceso -o Procesador.cpp
g++ Porcesador.cpp -o proceso
g++ Procesador.cpp -o proceso
./processor  Images/lena.ppm  lena2.ppm
./proceso  Images/lena.ppm  lena2.ppm
./proceso  Images/lena.pgm  lena2.ppm
./proceso  Images/lena.pgm  lena2.pgm
./proceso  Images/lena.ppm  Output_Images/lena2.ppm
./proceso  Images/lena.ppm  Ouput_Images/lena2.ppm
clear
vim filterer.cpp
g++ filterer.cpp -o filtro
./filtro Images/fruit.ppm Ouput_Images/fruit_blur.ppm
clear
./filtro Images/fruit.ppm Ouput_Images/fruit_blur.ppm  --f blur
clear
./filtro Images/fruit.ppm Ouput_Images/fruit_blur.ppm
clear
g++ filterer.cpp -o filtro
./filtro Images/fruit.ppm Ouput_Images/fruit_blur.ppm --f blur
./filtro Images/fruit.ppm Ouput_Images/fruit_blur.ppm --f laplace
./filtro Images/fruit.ppm Ouput_Images/fruit_blur.ppm --f blur
./filtro Images/fruit.ppm Ouput_Images/fruit_laplace.ppm --f blur
./filtro Images/fruit.ppm Ouput_Images/fruit_laplace.ppm --f laplace
./filtro Images/fruit.ppm Ouput_Images/fruit_sharpering.ppm --f sharpering
./filtro Images/fruit.ppm Ouput_Images/fruit_sharpering.ppm --f sharpen
