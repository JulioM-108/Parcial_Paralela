#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <omp.h>
using namespace std;

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

    const char* getMagic() const { 
        return magic;
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
            if ((i + 1) % 12 == 0) fprintf(out, "\n");
        }

        fclose(out);
        return true;
    }

    void applyKernel(const float kernel[3][3]) {
        int* result_pixels = (int*) malloc(pixel_count * sizeof(int));
        if (!result_pixels) {
            cerr << "Error reservando memoria para el filtro" << endl;
            return;
        }

        int channels = (strcmp(magic, "P3") == 0) ? 3 : 1;

        #pragma omp parallel for collapse(2)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int c = 0; c < channels; c++) {
                    float sum = 0.0, weight_sum = 0.0;

                    for (int ky = -1; ky <= 1; ky++) {
                        for (int kx = -1; kx <= 1; kx++) {
                            int nx = x + kx, ny = y + ky;
                            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                int idx = (ny * width + nx) * channels + c;
                                sum += pixels[idx] * kernel[ky + 1][kx + 1];
                                weight_sum += kernel[ky + 1][kx + 1];
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

    void applyBlur() {
        const float kernel[3][3] = {
            {1.0 / 9, 1.0 / 9, 1.0 / 9},
            {1.0 / 9, 1.0 / 9, 1.0 / 9},
            {1.0 / 9, 1.0 / 9, 1.0 / 9}
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
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " <input_image> <output_prefix>\n";
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_prefix = argv[2];

    char out_blur[256], out_laplace[256], out_sharpen[256];

    clock_t start_time = clock();

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            PNMImage img;
            if (img.load(input_file)) {
                const char* ext = (strcmp(img.getMagic(), "P3") == 0) ? ".ppm" : ".pgm";
                snprintf(out_blur, sizeof(out_blur), "%s_blur%s", output_prefix, ext);
                img.applyBlur();
                img.save(out_blur);
                cout << "Filtro blur aplicado y guardado en " << out_blur << endl;
            }
        }

        #pragma omp section
        {
            PNMImage img;
            if (img.load(input_file)) {
                const char* ext = (strcmp(img.getMagic(), "P3") == 0) ? ".ppm" : ".pgm";
                snprintf(out_laplace, sizeof(out_laplace), "%s_laplace%s", output_prefix, ext);
                img.applyLaplace();
                img.save(out_laplace);
                cout << "Filtro laplace aplicado y guardado en " << out_laplace << endl;
            }
        }

        #pragma omp section
        {
            PNMImage img;
            if (img.load(input_file)) {
                const char* ext = (strcmp(img.getMagic(), "P3") == 0) ? ".ppm" : ".pgm";
                snprintf(out_sharpen, sizeof(out_sharpen), "%s_sharpen%s", output_prefix, ext);
                img.applySharpen();
                img.save(out_sharpen);
                cout << "Filtro sharpen aplicado y guardado en " << out_sharpen << endl;
            }
        }
    }

    clock_t end_time = clock();
    double cpu_time = double(end_time - start_time) / CLOCKS_PER_SEC;
    cout << "Tiempo total con OpenMP: " << cpu_time << " segundos" << endl;

    return 0;
}
