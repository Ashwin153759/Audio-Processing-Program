#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    
    //check for the correct number of command line arguments
    if (argc!=3) {
        printf("Usage: %s <source_wav> <dest_wav>\n", argv[0]);
        return 1;
    }

    // Open the source file in binary read mode
    FILE *source_file = fopen(argv[1], "rb");
    //check if source file can be opened correctly
    if (!source_file) {
        printf("Error: Cannot open source file %s\n", argv[1]);
        return 1;
    }

    // Open the destination file in binary write mode
    FILE *dest_file = fopen(argv[2], "wb");
    //check if detination file can be opened correctly
    if (!dest_file) {
        printf("Error: Cannot open destination file %s\n", argv[2]);
        fclose(source_file);
        return 1;
    }


    //copy the first 44 bytes of input file to header_info
    char header_info[44];
    if (fread(header_info, 1, 44, source_file)!=44) {
        printf("File reading error has occured. Check for error, or end of file has been reached");
        fclose(source_file);
        fclose(dest_file);
        return 1;
    }
    //copy bytes from header_info to the output file
    if (fwrite(header_info, 1, 44, dest_file)!=44) {
        printf("File writing error has occured. Check for error.");
        fclose(source_file);
        fclose(dest_file);
        return 1;
    }

    //successfully copied header_info to the output file
    //now change the rest of the bytes and add them to the output file
    short left;
    short right; 
    short combined;
    //calculate the combined and add it to the destination file twice to keep the same format
    while (fread(&left, sizeof(short), 1, source_file)==1 && fread(&right, sizeof(short), 1, source_file)==1) {
        combined= (left-right)/2;
        fwrite(&combined, sizeof(short), 1, dest_file);
        fwrite(&combined, sizeof(short), 1, dest_file);
    }

    fclose(source_file);
    fclose(dest_file);

    return 0;
} 
