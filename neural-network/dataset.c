#include "dataset.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Dataset* alloc_dataset(int num_samples, int input_size, int output_size) {
    Dataset* dataset = (Dataset*)malloc(sizeof(Dataset));
    dataset->num_samples = num_samples;
    dataset->input_size = input_size;
    dataset->output_size = output_size;
    dataset->inputs = (double**)malloc(num_samples * sizeof(double*));
    dataset->targets = (double**)malloc(num_samples * sizeof(double*));
    return dataset;
}

// create sine wave dataset: z = sin(x) * cos(y)
Dataset* create_sine_wave_dataset(int num_samples) {
    Dataset* dataset = alloc_dataset(num_samples, 2, 1);

    for (int i = 0; i < num_samples; i++) {
        dataset->inputs[i] = (double*)malloc(2 * sizeof(double));
        dataset->targets[i] = (double*)malloc(1 * sizeof(double));

        // generate random points in [-π, π]
        double x = (rand() / (double)RAND_MAX) * 2 * M_PI - M_PI;
        double y = (rand() / (double)RAND_MAX) * 2 * M_PI - M_PI;

        dataset->inputs[i][0] = x;
        dataset->inputs[i][1] = y;

        // target: sin(x) * cos(y), normalized to [0, 1]
        double z = sin(x) * cos(y);
        dataset->targets[i][0] = (z + 1.0) / 2.0;
    }

    return dataset;
}

// create xor dataset with 100 samples and noise
Dataset* create_xor_dataset() {
    Dataset* dataset = alloc_dataset(100, 2, 1);

    for (int i = 0; i < dataset->num_samples; i++) {
        dataset->inputs[i] = (double*)malloc(2 * sizeof(double));
        dataset->targets[i] = (double*)malloc(1 * sizeof(double));

        int pattern = i % 4;
        double noise1 = ((rand() / (double)RAND_MAX) * 0.2 - 0.1);
        double noise2 = ((rand() / (double)RAND_MAX) * 0.2 - 0.1);

        switch (pattern) {
            case 0: // 0,0 -> 0
                dataset->inputs[i][0] = 0.0 + noise1;
                dataset->inputs[i][1] = 0.0 + noise2;
                dataset->targets[i][0] = 0.0;
                break;
            case 1: // 0,1 -> 1
                dataset->inputs[i][0] = 0.0 + noise1;
                dataset->inputs[i][1] = 1.0 + noise2;
                dataset->targets[i][0] = 1.0;
                break;
            case 2: // 1,0 -> 1
                dataset->inputs[i][0] = 1.0 + noise1;
                dataset->inputs[i][1] = 0.0 + noise2;
                dataset->targets[i][0] = 1.0;
                break;
            case 3: // 1,1 -> 0
                dataset->inputs[i][0] = 1.0 + noise1;
                dataset->inputs[i][1] = 1.0 + noise2;
                dataset->targets[i][0] = 0.0;
                break;
        }
    }

    return dataset;
}

// create circle dataset: uniform points in [-1,1]^2, inside r=0.7 -> 1
Dataset* create_circle_dataset(int num_samples) {
    // same generation stream as enhanced with zero boundary-focused samples
    return create_circle_dataset_enhanced(num_samples, 0.0);
}

// create circle dataset with more points near the decision boundary
Dataset* create_circle_dataset_enhanced(int num_samples, double boundary_ratio) {
    Dataset* dataset = alloc_dataset(num_samples, 2, 1);

    int boundary_samples = (int)(num_samples * boundary_ratio);
    int uniform_samples = num_samples - boundary_samples;

    double radius = 0.7;
    double boundary_width = 0.15; // points within radius ± boundary_width

    for (int i = 0; i < num_samples; i++) {
        dataset->inputs[i] = (double*)malloc(2 * sizeof(double));
        dataset->targets[i] = (double*)malloc(1 * sizeof(double));

        double x, y, distance;

        if (i < uniform_samples) {
            // uniform random samples
            x = (rand() / (double)RAND_MAX) * 2 - 1;
            y = (rand() / (double)RAND_MAX) * 2 - 1;
        } else {
            // samples near the decision boundary
            double angle = (rand() / (double)RAND_MAX) * 2 * M_PI;
            double r = radius + ((rand() / (double)RAND_MAX) * 2 - 1) * boundary_width;
            x = r * cos(angle);
            y = r * sin(angle);
        }

        dataset->inputs[i][0] = x;
        dataset->inputs[i][1] = y;

        // classify as 1 if inside circle of radius 0.7, else 0
        distance = sqrt(x * x + y * y);
        if (distance < radius) {
            dataset->targets[i][0] = 1.0;
        } else {
            dataset->targets[i][0] = 0.0;
        }
    }

    return dataset;
}

Dataset* load_csv_dataset(const char* filepath, int input_size, int output_size, int has_header) {
    if (!filepath || input_size <= 0 || output_size <= 0)
        return NULL;

    FILE* fp = fopen(filepath, "r");
    if (!fp)
        return NULL;

    char line[4096];
    int line_count = 0;

    if (has_header && fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char* p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        if (*p != '\0')
            line_count++;
    }

    if (line_count == 0) {
        fclose(fp);
        return NULL;
    }

    rewind(fp);
    if (has_header) {
        if (fgets(line, sizeof(line), fp) == NULL) {
            fclose(fp);
            return NULL;
        }
    }

    Dataset* dataset = alloc_dataset(line_count, input_size, output_size);

    int current_sample = 0;
    while (fgets(line, sizeof(line), fp) != NULL && current_sample < line_count) {
        char* p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        if (*p == '\0')
            continue;

        dataset->inputs[current_sample] = (double*)malloc(input_size * sizeof(double));
        dataset->targets[current_sample] = (double*)malloc(output_size * sizeof(double));

        char* token = strtok(line, ",\t\r\n");
        int col = 0;
        while (token && col < input_size + output_size) {
            double val = atof(token);
            if (col < input_size) {
                dataset->inputs[current_sample][col] = val;
            } else {
                dataset->targets[current_sample][col - input_size] = val;
            }
            col++;
            token = strtok(NULL, ",\t\r\n");
        }
        current_sample++;
    }

    fclose(fp);
    return dataset;
}

// shuffled index array over n samples (fisher-yates)
static int* shuffled_indices(int n) {
    int* indices = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
        indices[i] = i;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    return indices;
}

static Dataset* alloc_subset(const Dataset* src, int num_samples) {
    return alloc_dataset(num_samples, src->input_size, src->output_size);
}

// copy rows [from, from+num) of shuffled indices into dst (deep-copies samples)
static void copy_rows(Dataset* dst, const Dataset* src, const int* indices, int from, int num) {
    for (int i = 0; i < num; i++) {
        int idx = indices[from + i];
        dst->inputs[i] = (double*)malloc(src->input_size * sizeof(double));
        dst->targets[i] = (double*)malloc(src->output_size * sizeof(double));
        for (int j = 0; j < src->input_size; j++)
            dst->inputs[i][j] = src->inputs[idx][j];
        for (int j = 0; j < src->output_size; j++)
            dst->targets[i][j] = src->targets[idx][j];
    }
}

void split_dataset(Dataset* dataset, double train_ratio,
                   Dataset** train_set, Dataset** test_set) {

    if (train_ratio < 0 || train_ratio > 1)
        train_ratio = 0.8;

    int train_size = (int)(dataset->num_samples * train_ratio);
    int test_size = dataset->num_samples - train_size;

    *train_set = alloc_subset(dataset, train_size);
    *test_set = alloc_subset(dataset, test_size);

    int* indices = shuffled_indices(dataset->num_samples);
    copy_rows(*train_set, dataset, indices, 0, train_size);
    copy_rows(*test_set, dataset, indices, train_size, test_size);
    free(indices);
}

void split_dataset_three_way(Dataset* dataset, double train_ratio, double val_ratio,
                             Dataset** train_set, Dataset** val_set, Dataset** test_set) {

    if (train_ratio < 0 || train_ratio > 1)
        train_ratio = 0.7;
    if (val_ratio < 0 || val_ratio > 1)
        val_ratio = 0.15;
    if (train_ratio + val_ratio >= 1.0) {
        train_ratio = 0.7;
        val_ratio = 0.15;
    }

    int train_size = (int)(dataset->num_samples * train_ratio);
    int val_size = (int)(dataset->num_samples * val_ratio);
    int test_size = dataset->num_samples - train_size - val_size;

    *train_set = alloc_subset(dataset, train_size);
    *val_set = alloc_subset(dataset, val_size);
    *test_set = alloc_subset(dataset, test_size);

    int* indices = shuffled_indices(dataset->num_samples);
    copy_rows(*train_set, dataset, indices, 0, train_size);
    copy_rows(*val_set, dataset, indices, train_size, val_size);
    copy_rows(*test_set, dataset, indices, train_size + val_size, test_size);
    free(indices);
}

void free_dataset(Dataset* dataset) {
    if (!dataset)
        return;

    for (int i = 0; i < dataset->num_samples; i++) {
        free(dataset->inputs[i]);
        free(dataset->targets[i]);
    }
    free(dataset->inputs);
    free(dataset->targets);
    free(dataset);
}
