#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//add the header to the output file with the size changes
void update_header_sizes(short *header, int delay) {
        // Treat the memory starting at address header + 2 as if it were an unsigned int
        unsigned int *first_chunk = (unsigned int *)(header + 2);
        unsigned int *second_chunk = (unsigned int *)(header + 20);

        // Update the sizes to account for the delay
        *first_chunk += delay * 2;
        *second_chunk += delay * 2;
}

//get the samples that are to be delayed
short* read_samples(FILE *source_file, int delay) {
    // Allocate memory to hold the delay samples.
    short *delay_samples = (short *)malloc(delay * sizeof(short));
    if (!delay_samples) {
        perror("Memory allocation failed for delay_samples");
        exit(EXIT_FAILURE);
    }

    // Read delay number of samples from the source file into the buffer.
    int samples_read = fread(delay_samples, sizeof(short), delay, source_file);

    // If there were not enough samples, fill the rest with zeroes.
    for (int i = samples_read; i < delay; i++) {
        delay_samples[i] = 0;
    }

    // Return the buffer containing the samples.
    return delay_samples;
}

int main(int argc, char** argv) {
    // default value for delay and scale
    long delay = 8000;
    long volume_scale = 4;
    
    // command line variable
    int opt;

    // check for valid command length
    if (argc < 3 || argc > 7 || argc%2 == 0) {
        printf("Usage: %s [-d delay] [-v volume_scale] sourcewav destwav\n", argv[0]);
        return 1;
    }

    // go through the command line
    while ((opt = getopt(argc, argv, ":d:v:")) != -1) {
        // cases
        switch(opt) {
            case 'd': 
                delay = strtol(optarg, NULL, 10);
                break;
            case 'v':
                volume_scale = strtol(optarg, NULL, 10);
                break;
        }
    }

    if (delay<=0 || volume_scale <=0) {
        printf("Invalid Delay or Volume Amounts, Please Enter Positive Numbers");
        return 1;
    }



    // index for file location
    int source = optind;
    int dest = optind + 1;

    // Open the source file in binary read mode
    FILE *source_file = fopen(argv[source], "rb");
    //check if source file can be opened correctly
    if (!source_file) {
        printf("Error: Cannot open source file %s\n", argv[source]);
        return 1;
    }

    // Open the destination file in binary write mode
    FILE *dest_file = fopen(argv[dest], "wb");
    //check if detination file can be opened correctly
    if (!dest_file) {
        printf("Error: Cannot open destination file %s\n", argv[dest]);
        fclose(source_file);
        return 1;
    }

    // header
    short header_info[22];
    // Read header from input file
    fread(header_info, sizeof(short), 22, source_file);
    update_header_sizes(header_info, delay);

    // Write the updated header to the destination file
    fwrite(header_info, sizeof(short), 22, dest_file);

    // get the samples that are to be delayed, then write the samples to the destination file
    short *saved_samples = read_samples(source_file, delay);
    fwrite(saved_samples, sizeof(short), delay, dest_file);

    // Buffer for new samples from the source file.
    short *new_samples = (short *)malloc(delay * sizeof(short));
    // Number of samples read from the source file.
    int samples_read = fread(new_samples, sizeof(short), delay, source_file);
    int last_amount_read;
    // if delay > size of the file
    if (samples_read == 0) {
        fseek(source_file, 44, SEEK_SET);
        last_amount_read = fread(new_samples, sizeof(short), delay, source_file);
    }



    // Process the entire source file.
    while (samples_read > 0) {
        // Mix the new_samples with the saved_samples.
        if (samples_read==delay) {
            for (int i = 0; i < samples_read; i++) {
                // Apply the mixing logic (here it's just a direct sum of the samples).
                short mixed_sample = new_samples[i] + saved_samples[i]/volume_scale;
                // Write the mixed sample to the destination file.
                fwrite(&mixed_sample, sizeof(short), 1, dest_file);
            }

            // Move the new_samples to saved_samples for the next round.
            memcpy(saved_samples, new_samples, samples_read * sizeof(short));
        }
        else {
            for (int i=0; i<samples_read; i++) {
                //write the mixed sample to destination file
                short mixed_sample = new_samples[i] + saved_samples[i]/volume_scale;
                fwrite(&mixed_sample, sizeof(short), 1, dest_file);
            }
            // If fewer samples were read than delay, fill the rest of saved_samples with zeros.
            for (int i = samples_read; i < delay; i++) {
                short mixed_sample = new_samples[i]/volume_scale;
                fwrite(&mixed_sample, sizeof(short), 1, dest_file);
            }
            last_amount_read = samples_read;
        }
        samples_read = fread(new_samples, sizeof(short), delay, source_file);
    }

    // Once the file is done, write any remaining echo.
    for (int i=0; i<last_amount_read; i++){
        short mixed_sample = new_samples[i]/volume_scale;
        fwrite(&mixed_sample, sizeof(short), 1, dest_file);
    }

    fclose(source_file);
    fclose(dest_file);

    return 0;  
} 