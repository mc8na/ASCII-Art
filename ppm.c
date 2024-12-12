#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
    unsigned char c;
} TXTPixel;

typedef struct {
     int x, y;
     PPMPixel* data;
} PPMImage;

typedef struct {
    int x, y;
    TXTPixel* data;
} TXTImage;

char characters[] = {'`','.','-',':','|','+','x','=','X','#','@'};

double maxBrightness = 0.0;
double minBrightness = 255.0;

bool contrast;

#define RGB_COMPONENT_COLOR 255

PPMImage* readPPM(const char *filename)
{
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    
    //open PPM file for reading
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
         fprintf(stderr, "Invalid image format (must be 'P6')\n");
         exit(1);
    }

    //alloc memory for image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n');
        c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    return img;
}

void writeTXT(const char* filename, TXTImage* img) 
{
    FILE* fp;
    int offset = 0;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    for (int i = 0; i < img->y; ++i) {
        fwrite(img->data+(offset), 1, img->x, fp);
        fprintf(fp, "\n");
        offset += img->x;
    }

    fclose(fp);
}

TXTPixel convertPPMPixelToTXTPixel(PPMPixel ppmpix)
{
    double brightness = 0.299*ppmpix.red + 0.587*ppmpix.green + 0.114*ppmpix.blue;
    brightness /= 25.5;

    int idx;
    if (contrast)
        idx = (int) brightness;
    else
        idx = 10-(int)brightness;

    TXTPixel txtpix;
    txtpix.c = characters[idx];
    return txtpix;
}

TXTImage* convertPPMImageToTXTImage(PPMImage* img)
{
    TXTImage* txt = (TXTImage*)malloc(sizeof(TXTImage));
    if (img) {
        txt->x = img->x;
        txt->y = img->y;
        txt->data = (TXTPixel*)malloc(img->x * img->y * sizeof(TXTPixel));
        for (int i = 0; i < img->x*img->y; ++i) {
            txt->data[i] = convertPPMPixelToTXTPixel(img->data[i]);
        }
    }
    return txt;
}

int main(int argc, char* argv[]){
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_ppm_file> <output_txt_file> <0/1>\n", argv[0]);
        return 1;
    }

    const char* inputFileName = argv[1];
    const char* outputFileName = argv[2];
    int contrastFlag = atoi(argv[3]);

    if (contrastFlag != 0 && contrastFlag != 1) {
        fprintf(stderr, "Error: contrast_flag must be 0 or 1\n");
        return 1;
    } else
        contrast = (contrastFlag == 1);

    PPMImage* ppmImage;
    TXTImage* txtImage;

    ppmImage = readPPM(inputFileName);
    txtImage = convertPPMImageToTXTImage(ppmImage);
    writeTXT(outputFileName,txtImage);
    
    free(ppmImage);
    free(txtImage);
}
