#include <mpi.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <ctime>

using namespace std;


bool load_pnm(const char* filename, char magic[3], int &width, int &height, int &max_color, int* &pixels, int &pixel_count) {
    FILE* f = fopen(filename, "r");
    if (!f) { cerr << "Error abriendo " << filename << "\n"; return false; }
    if (fscanf(f, "%2s", magic) != 1) { fclose(f); return false; }

   
    int c;
    auto skip_ws_comments = [&](FILE* fp) {
        while ((c = fgetc(fp)) != EOF) {
            if (isspace(c)) continue;
            if (c == '#') { 
                while ((c = fgetc(fp)) != EOF && c != '\n') {}
                continue;
            }
            ungetc(c, fp);
            break;
        }
    };

    skip_ws_comments(f);
    if (fscanf(f, "%d %d", &width, &height) != 2) { fclose(f); return false; }

    skip_ws_comments(f);
    if (fscanf(f, "%d", &max_color) != 1) { fclose(f); return false; }

    pixel_count = width * height;
    if (strcmp(magic, "P3") == 0) pixel_count *= 3;

    pixels = (int*) malloc(pixel_count * sizeof(int));
    if (!pixels) { fclose(f); return false; }

    for (int i = 0; i < pixel_count; ++i) {
        skip_ws_comments(f);
        if (fscanf(f, "%d", &pixels[i]) != 1) {
            free(pixels);
            fclose(f);
            return false;
        }
    }

    fclose(f);
    return true;
}


bool save_pnm(const char* filename, const char magic[3], int width, int height, int max_color, const int* pixels, int pixel_count) {
    FILE* f = fopen(filename, "w");
    if (!f) { cerr << "Error abriendo para escribir " << filename << "\n"; return false; }
    fprintf(f, "%s\n%d %d\n%d\n", magic, width, height, max_color);
    for (int i = 0; i < pixel_count; ++i) {
        fprintf(f, "%d ", pixels[i]);
        if ((i+1) % 12 == 0) fprintf(f, "\n");
    }
    fprintf(f, "\n");
    fclose(f);
    return true;
}


void apply_kernel_block(const int* in_pixels, int* out_pixels,
                        int width, int height, int channels, int max_color,
                        const float kernel[3][3]) {

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                float sum = 0.0f;
                float weight_sum = 0.0f;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        int nx = x + kx;
                        int ny = y + ky;
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            int idx = (ny * width + nx) * channels + c;
                            sum += in_pixels[idx] * kernel[ky+1][kx+1];
                            weight_sum += kernel[ky+1][kx+1];
                        }
                    }
                }
                float value = (weight_sum != 0.0f) ? sum / weight_sum : sum;
                int result = static_cast<int>(value + 0.5f);
                if (result < 0) result = 0;
                if (result > max_color) result = max_color;
                int idx = (y * width + x) * channels + c;
                out_pixels[idx] = result;
            }
        }
    }
}


int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    if (argc < 4) {
        if (rank == 0) {
            cout << "Uso: " << argv[0] << " <input_image> <output_prefix> --f <filter>\n";
            cout << "Ejemplo (ejecutar con mpirun): mpirun -np 4 ./mpi_filterer sulfur.pgm sulfur --f blur\n";
        }
        MPI_Finalize();
        return 1;
    }

    const char* input = argv[1];
    const char* outprefix = argv[2];

    
    if (rank == 0) {
        if (argc < 5 || strcmp(argv[3], "--f") != 0) {
            cerr << "Error: se esperaba --f <filter>\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    char filter_name[32] = {0};
    if (rank == 0) strncpy(filter_name, argv[4], 31);
    MPI_Bcast(filter_name, 32, MPI_CHAR, 0, MPI_COMM_WORLD);

   
    float blur_kernel[3][3] = {{1.0f/9,1.0f/9,1.0f/9},{1.0f/9,1.0f/9,1.0f/9},{1.0f/9,1.0f/9,1.0f/9}};
    float laplace_kernel[3][3] = {{0,-1,0},{-1,4,-1},{0,-1,0}};
    float sharpen_kernel[3][3] = {{0,-1,0},{-1,5,-1},{0,-1,0}};
    const float (*kernel)[3] = nullptr;

    if (strcmp(filter_name, "blur") == 0) kernel = blur_kernel;
    else if (strcmp(filter_name, "laplace") == 0) kernel = laplace_kernel;
    else if (strcmp(filter_name, "sharpen") == 0) kernel = sharpen_kernel;
    else {
        if (rank == 0) cerr << "Filtro no reconocido: " << filter_name << "\n";
        MPI_Abort(MPI_COMM_WORLD,1);
    }

    
    char magic[3] = {'\0','\0','\0'};
    int width = 0, height = 0, max_color = 0;
    int channels = 1;
    int pixel_count = 0;
    int* pixels = nullptr;

    if (rank == 0) {
        if (!load_pnm(input, magic, width, height, max_color, pixels, pixel_count)) {
            cerr << "Rank 0: error cargando imagen " << input << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    
    MPI_Bcast(magic, 3, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_color, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        channels = (strcmp(magic, "P3") == 0) ? 3 : 1;
        pixel_count = width * height * channels;
        pixels = (int*) malloc(pixel_count * sizeof(int));
        if (!pixels) { cerr << "Rank " << rank << ": malloc failed\n"; MPI_Abort(MPI_COMM_WORLD,1); }
    } else {
        channels = (strcmp(magic, "P3") == 0) ? 3 : 1;
    }

    
    MPI_Bcast(pixels, pixel_count, MPI_INT, 0, MPI_COMM_WORLD);


    int* out_pixels = (int*) malloc(pixel_count * sizeof(int));
    if (!out_pixels) { cerr << "Rank " << rank << ": output malloc failed\n"; MPI_Abort(MPI_COMM_WORLD,1); }

    
    double t0 = MPI_Wtime();
    clock_t c0 = clock();

 
    apply_kernel_block(pixels, out_pixels, width, height, channels, max_color, kernel);

    clock_t c1 = clock();
    double t1 = MPI_Wtime();

    double cpu_time = double(c1 - c0) / CLOCKS_PER_SEC;
    double wall_time = t1 - t0;

   
    char outname[512];
    snprintf(outname, sizeof(outname), "%s_rank%d.ppm", outprefix, rank);
    if (!save_pnm(outname, magic, width, height, max_color, out_pixels, pixel_count)) {
        cerr << "Rank " << rank << ": error saving " << outname << "\n";
        
    } else {
        if (rank == 0) {
            cout << "Rank " << rank << ": wrote output " << outname << "\n";
        }
    }

    
    double local_times[2] = { cpu_time, wall_time };
    double* all_times = nullptr;
    if (rank == 0) all_times = (double*) malloc(world * 2 * sizeof(double));
    MPI_Gather(local_times, 2, MPI_DOUBLE, all_times, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
      
        cout << "=== Tiempos por nodo ===\n";
        for (int r = 0; r < world; ++r) {
            double ctime_r = all_times[r*2 + 0];
            double wtime_r = all_times[r*2 + 1];
            cout << "Rank " << r << ": CPU time = " << ctime_r << " s, wall time = " << wtime_r << " s\n";
        }
        free(all_times);
    }

    free(pixels);
    free(out_pixels);

    MPI_Finalize();
    return 0;
}
