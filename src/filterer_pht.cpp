#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <pthread.h>
using namespace std;


int* g_pixels;
int g_width, g_height, g_channels, g_max_color;
const float (*g_kernel)[3];


void* applyKernelRegion(void* arg) {
    int thread_id = *((int*)arg); 

    int mid_x = g_width / 2;
    int mid_y = g_height / 2;

    int start_x, end_x, start_y, end_y;

    
    if (thread_id == 0) { 
        start_x = 0; end_x = mid_x;
        start_y = 0; end_y = mid_y;
    } else if (thread_id == 1) { 
        start_x = mid_x; end_x = g_width;
        start_y = 0; end_y = mid_y;
    } else if (thread_id == 2) { 
        start_x = 0; end_x = mid_x;
        start_y = mid_y; end_y = g_height;
    } else { 
        start_x = mid_x; end_x = g_width;
        start_y = mid_y; end_y = g_height;
    }


    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            for (int c = 0; c < g_channels; c++) {
                float sum = 0.0, weight_sum = 0.0;

                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int nx = x + kx, ny = y + ky;
                        if (nx >= 0 && nx < g_width && ny >= 0 && ny < g_height) {
                            int idx = (ny * g_width + nx) * g_channels + c;
                            sum += g_pixels[idx] * g_kernel[ky+1][kx+1];
                            weight_sum += g_kernel[ky+1][kx+1];
                        }
                    }
                }

                int idx = (y * g_width + x) * g_channels + c;
                float value = (weight_sum != 0) ? sum / weight_sum : sum;
                g_pixels[idx] = max(0, min(g_max_color, (int)value));
            }
        }
    }

    pthread_exit(nullptr);
}


class PNMImage {
private:
    char magic[3];
    int width;
    int height;
    int max_color;
    int* pixels;
    int pixel_count;

public:
    PNMImage() : width(0), height(0), max_color(0), pixels(nullptr), pixel_count(0) {
        magic[0] = '\0';
    }

    ~PNMImage() {
        if (pixels) free(pixels);
    }

    bool load(const char* filename) {
        FILE* file = fopen(filename, "r");
        if (!file) {
            cerr << "Error: no se pudo abrir " << filename << endl;
            return false;
        }

        if (fscanf(file, "%2s", magic) != 1) {
            cerr << "Error leyendo magic number" << endl;
            fclose(file);
            return false;
        }

        if (fscanf(file, "%d %d", &width, &height) != 2) {
            cerr << "Error leyendo width/height" << endl;
            fclose(file);
            return false;
        }

        if (fscanf(file, "%d", &max_color) != 1) {
            cerr << "Error leyendo max_color" << endl;
            fclose(file);
            return false;
        }

        pixel_count = width * height;
        if (strcmp(magic, "P3") == 0) pixel_count *= 3;

        pixels = (int*) malloc(pixel_count * sizeof(int));
        if (!pixels) {
            cerr << "Error reservando memoria" << endl;
            fclose(file);
            return false;
        }

        for (int i = 0; i < pixel_count; i++) {
            if (fscanf(file, "%d", &pixels[i]) != 1) {
                cerr << "Error leyendo píxeles" << endl;
                free(pixels);
                pixels = nullptr;
                fclose(file);
                return false;
            }
        }

        fclose(file);
        return true;
    }

    bool save(const char* filename) const {
        FILE* out = fopen(filename, "w");
        if (!out) {
            cerr << "Error: no se pudo abrir " << filename << " para escritura" << endl;
            return false;
        }

        fprintf(out, "%s\n%d %d\n%d\n", magic, width, height, max_color);

        for (int i = 0; i < pixel_count; i++) {
            fprintf(out, "%d ", pixels[i]);
            if ((i+1) % 12 == 0) fprintf(out, "\n");
        }

        fclose(out);
        return true;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getMaxColor() const { return max_color; }
    int* getPixels() { return pixels; }
    const char* getMagic() const { return magic; }
};


int main(int argc, char* argv[]) {
    if (argc < 5) {
        cout << "Uso: " << argv[0] << " <input_image> <output_image> --f <filtro>\n";
        cout << "Filtros disponibles: blur, laplace, sharpen\n";
        return 1;
    }

    PNMImage img;
    if (!img.load(argv[1])) return 1;

    if (strcmp(argv[3], "--f") != 0) {
        cerr << "Error: se esperaba la bandera --f\n";
        return 1;
    }


    const float blur_kernel[3][3] = {
        {1.0/9, 1.0/9, 1.0/9},
        {1.0/9, 1.0/9, 1.0/9},
        {1.0/9, 1.0/9, 1.0/9}
    };
    const float laplace_kernel[3][3] = {
        {0, -1, 0},
        {-1, 4, -1},
        {0, -1, 0}
    };
    const float sharpen_kernel[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
    };

    if (strcmp(argv[4], "blur") == 0) g_kernel = blur_kernel;
    else if (strcmp(argv[4], "laplace") == 0) g_kernel = laplace_kernel;
    else if (strcmp(argv[4], "sharpen") == 0) g_kernel = sharpen_kernel;
    else {
        cerr << "Filtro no reconocido: " << argv[4] << endl;
        return 1;
    }


    g_width = img.getWidth();
    g_height = img.getHeight();
    g_channels = (strcmp(img.getMagic(), "P3") == 0) ? 3 : 1;
    g_max_color = img.getMaxColor();
    g_pixels = img.getPixels();


    clock_t start_time = clock();

    pthread_t threads[4];
    int ids[4] = {0,1,2,3};

    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], nullptr, applyKernelRegion, &ids[i]);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], nullptr);
    }

    clock_t end_time = clock();
    double cpu_time = double(end_time - start_time) / CLOCKS_PER_SEC;
    cout << "Tiempo de CPU con pthreads: " << cpu_time << " segundos" << endl;

    if (!img.save(argv[2])) return 1;
    cout << "Imagen procesada con filtro " << argv[4]
         << " y guardada en " << argv[2] << endl;

    return 0;
}
