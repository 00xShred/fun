#ifndef DATASET_H
#define DATASET_H

typedef struct {
    double** inputs;
    double** targets;
    int num_samples;
    int input_size;
    int output_size;
} Dataset;

Dataset* create_sine_wave_dataset(int num_samples);
Dataset* create_xor_dataset();
Dataset* create_circle_dataset(int num_samples);
Dataset* create_circle_dataset_enhanced(int num_samples, double boundary_ratio);
Dataset* load_csv_dataset(const char* filepath, int input_size, int output_size, int has_header);

void split_dataset(Dataset* dataset, double train_ratio,
                   Dataset** train_set, Dataset** test_set);
void split_dataset_three_way(Dataset* dataset, double train_ratio, double val_ratio,
                             Dataset** train_set, Dataset** val_set, Dataset** test_set);
void free_dataset(Dataset* dataset);

#endif
