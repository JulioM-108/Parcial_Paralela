#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <ctime>   
using namespace std;

class PNMImage {
private:
    char magic[3];
    int width;
    int height;
    int max_color;
    int* pixels;
    int pixel_count;

    void applyKernel(const float kernel[3][3]) {
        int* result_pixels = (int*) malloc(pixel_count * sizeof(int));
        if (!result_pixels) {
            cerr << "Error reservando memoria para el filtro" << endl;
            return;
        }

        int channels = (strcmp(magic, "P3") == 0) ? 3 : 1;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int c = 0; c < channels; c++) {
                    float sum = 0.0;
                    float weight_sum = 0.0;

                    for (int ky = -1; ky <= 1; ky++) {
                        for (int kx = -1; kx <= 1; kx++) {
                            int nx = x + kx;
                            int ny = y + ky;

                            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                int idx = (ny * width + nx) * channels + c;
                                sum += pixels[idx] * kernel[ky+1][kx+1];
                                weight_sum += kernel[ky+1][kx+1];
                            }
                        }
                    }

                    float value = (weight_sum != 0) ? sum / weight_sum : sum;
                    int result = static_cast<int>(value);

                    result = max(0, min(max_color, result));

                    int idx = (y * width + x) * channels + c;
                    result_pixels[idx] = result;
                }
            }
        }

        free(pixels);
        pixels = result_pixels;
    }

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
        if (strcmp(magic, "P3") == 0) {
            pixel_count *= 3;
        }

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

    void applyBlur() {
        const float kernel[3][3] = {
            {1.0/9, 1.0/9, 1.0/9},
            {1.0/9, 1.0/9, 1.0/9},
            {1.0/9, 1.0/9, 1.0/9}
        };
        applyKernel(kernel);
    }

    void applyLaplace() {
        const float kernel[3][3] = {
            {0, -1, 0},
            {-1, 4, -1},
            {0, -1, 0}
        };
        applyKernel(kernel);
    }

    void applySharpen() {
        const float kernel[3][3] = {
            {0, -1, 0},
            {-1, 5, -1},
            {0, -1, 0}
        };
        applyKernel(kernel);
    }
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

    clock_t start_time = clock();

    if (strcmp(argv[4], "blur") == 0) {
        img.applyBlur();
    } else if (strcmp(argv[4], "laplace") == 0) {
        img.applyLaplace();
    } else if (strcmp(argv[4], "sharpen") == 0) {
        img.applySharpen();
    } else {
        cerr << "Filtro no reconocido: " << argv[4] << endl;
        return 1;
    }

    clock_t end_time = clock();

    double cpu_time = double(end_time - start_time) / CLOCKS_PER_SEC;
    cout << "Tiempo de CPU usado en el filtrado: " << cpu_time << " segundos" << endl;

    if (!img.save(argv[2])) return 1;

    cout << "Imagen procesada con filtro " << argv[4]
         << " y guardada en " << argv[2] << endl;

    return 0;
}
