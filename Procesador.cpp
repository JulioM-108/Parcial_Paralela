#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

class PNMImage {
private:
    char magic[3];   // "P2" o "P3"
    int width;
    int height;
    int max_color;
    int* pixels;
    int pixel_count;

public:
    // Constructor
    PNMImage() : width(0), height(0), max_color(0), pixels(nullptr), pixel_count(0) {
        magic[0] = '\0';
    }

    // Destructor
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
                cerr << "Error leyendo pixeles" << endl;
                free(pixels);
                pixels = nullptr;
                fclose(file);
                return false;
            }
        }

        fclose(file);
        return true;
    }

    // Guardar imagen
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
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " input_image output_image" << endl;
        return 1;
    }

    PNMImage img;

    if (!img.load(argv[1])) return 1;
    if (!img.save(argv[2])) return 1;

    cout << "Imagen copiada exitosamente.\n";
    return 0;
}
