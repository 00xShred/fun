#include "dataset.h"
#include "neural_network.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
    int epochs;
    double learning_rate;
    int hidden_size;
    int batch_size;
    int verbose;
    int patience;
    double momentum;
    int gradient_check;
    int shuffle;

    double decay_rate;
    int decay_steps;

    char dataset_type[20];
    int dataset_size;
    double train_test_split;
    double boundary_ratio;

    char hidden_activation[20];
    char output_activation[20];

    char loss_function[20];

    char optimizer[20];

    double weight_decay;

    int use_validation_set;
    double validation_ratio;
    int seed;
    char save_path[128];
    char load_path[128];
    char csv_path[128];
    int csv_input_size;
    int csv_output_size;
    int csv_has_header;
} TrainingConfig;

void print_usage(const char *program_name);
TrainingConfig parse_arguments(int argc, char *argv[]);
void print_config(const TrainingConfig *config);

// print progress bar (non-verbose mode)
static void print_progress(int epoch, int total_epochs, double error) {
    int bar_width  = 50;
    float progress = (float)epoch / total_epochs;
    int pos        = bar_width * progress;

    printf("[");
    for (int i = 0; i < bar_width; i++) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf(
        "] %d%% | epoch: %d/%d | error: %.6f\r", (int)(progress * 100), epoch, total_epochs, error);
    fflush(stdout);
}

void print_usage(const char *program_name) {
    printf("usage: %s [options]\n", program_name);
    printf("options:\n");
    printf("  -e <epochs>     number of training epochs (default: 10000)\n");
    printf("  -l <rate>       learning rate (default: 0.1)\n");
    printf("  -h <size>       hidden layer size (default: 8)\n");
    printf("  -b <size>       batch size (default: 32)\n");
    printf("  -m <momentum>   momentum (default: 0.9)\n");
    printf("  -dr <rate>      learning rate decay rate (default: 0.995)\n");
    printf("  -ds <steps>     decay steps (default: 100)\n");
    printf("  -d <type>       dataset type: xor, sine, circle, circle_enhanced (default: sine)\n");
    printf("  -n <size>       dataset size (default: 1000)\n");
    printf("  -split <ratio>  train/test split ratio (default: 0.8)\n");
    printf(
        "  -ha <act>       hidden activation: sigmoid, relu, leaky_relu, tanh, linear (default: relu)\n");
    printf(
        "  -oa <act>       output activation: sigmoid, relu, leaky_relu, tanh, linear (default: sigmoid)\n");
    printf("  -loss <type>    loss function: mse, bce, mae, cce (default: mse)\n");
    printf("  -opt <type>     optimizer: sgd, adam (default: sgd)\n");
    printf("  -wd <decay>     weight decay (l2 regularization) (default: 0.0)\n");
    printf("  -br <ratio>     boundary ratio for enhanced circle dataset (default: 0.3)\n");
    printf("  -p <epochs>     early stopping patience (0 to disable, default: 50)\n");
    printf("  -val <ratio>    validation set ratio (default: 0.0)\n");
    printf("  -s              shuffle data each epoch (default: yes)\n");
    printf("  -ns             no shuffle\n");
    printf("  -seed <int>     random seed (-1 for time-based, default: -1)\n");
    printf("  -save <path>    save trained model to file\n");
    printf("  -load <path>    load model from file\n");
    printf("  -csv <path>     load dataset from CSV file\n");
    printf("  -csv-in <n>     number of CSV input columns (default: 2)\n");
    printf("  -csv-out <n>    number of CSV output columns (default: 1)\n");
    printf("  -csv-header     CSV file has header row\n");
    printf("  -g              enable gradient checking (before training)\n");
    printf("  -v              verbose mode\n");
    printf("  -?              show this help message\n");
}

TrainingConfig parse_arguments(int argc, char *argv[]) {
    TrainingConfig config;
    config.epochs         = 10000;
    config.learning_rate  = 0.1;
    config.hidden_size    = 8;
    config.batch_size     = 32;
    config.verbose        = 0;
    config.patience       = 50;
    config.momentum       = 0.9;
    config.gradient_check = 0;
    config.shuffle        = 1;

    config.decay_rate  = 0.995;
    config.decay_steps = 100;

    strcpy(config.dataset_type, "sine");
    config.dataset_size     = 1000;
    config.train_test_split = 0.8;
    config.boundary_ratio   = 0.3;

    strcpy(config.hidden_activation, "relu");
    strcpy(config.output_activation, "sigmoid");

    strcpy(config.loss_function, "mse");
    strcpy(config.optimizer, "sgd");

    config.weight_decay = 0.0;

    config.use_validation_set = 0;
    config.validation_ratio   = 0.0;
    config.seed               = -1;
    config.save_path[0]       = '\0';
    config.load_path[0]       = '\0';
    config.csv_path[0]        = '\0';
    config.csv_input_size     = 2;
    config.csv_output_size    = 1;
    config.csv_has_header     = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            config.epochs = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            config.learning_rate = atof(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            config.hidden_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            config.batch_size = atoi(argv[++i]);
            if (config.batch_size <= 0) config.batch_size = 1;
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            config.momentum = atof(argv[++i]);
        } else if (strcmp(argv[i], "-dr") == 0 && i + 1 < argc) {
            config.decay_rate = atof(argv[++i]);
        } else if (strcmp(argv[i], "-ds") == 0 && i + 1 < argc) {
            config.decay_steps = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            strncpy(config.dataset_type, argv[++i], 19);
            config.dataset_type[19] = '\0';
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            config.dataset_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-split") == 0 && i + 1 < argc) {
            config.train_test_split = atof(argv[++i]);
        } else if (strcmp(argv[i], "-ha") == 0 && i + 1 < argc) {
            strncpy(config.hidden_activation, argv[++i], 19);
            config.hidden_activation[19] = '\0';
        } else if (strcmp(argv[i], "-oa") == 0 && i + 1 < argc) {
            strncpy(config.output_activation, argv[++i], 19);
            config.output_activation[19] = '\0';
        } else if (strcmp(argv[i], "-loss") == 0 && i + 1 < argc) {
            strncpy(config.loss_function, argv[++i], 19);
            config.loss_function[19] = '\0';
        } else if (strcmp(argv[i], "-opt") == 0 && i + 1 < argc) {
            strncpy(config.optimizer, argv[++i], 19);
            config.optimizer[19] = '\0';
        } else if (strcmp(argv[i], "-wd") == 0 && i + 1 < argc) {
            config.weight_decay = atof(argv[++i]);
        } else if (strcmp(argv[i], "-br") == 0 && i + 1 < argc) {
            config.boundary_ratio = atof(argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            config.patience = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-val") == 0 && i + 1 < argc) {
            config.validation_ratio   = atof(argv[++i]);
            config.use_validation_set = (config.validation_ratio > 0.0);
        } else if (strcmp(argv[i], "-g") == 0) {
            config.gradient_check = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            config.shuffle = 1;
        } else if (strcmp(argv[i], "-ns") == 0) {
            config.shuffle = 0;
        } else if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            config.seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-save") == 0 && i + 1 < argc) {
            strncpy(config.save_path, argv[++i], sizeof(config.save_path) - 1);
            config.save_path[sizeof(config.save_path) - 1] = '\0';
        } else if (strcmp(argv[i], "-load") == 0 && i + 1 < argc) {
            strncpy(config.load_path, argv[++i], sizeof(config.load_path) - 1);
            config.load_path[sizeof(config.load_path) - 1] = '\0';
        } else if (strcmp(argv[i], "-csv") == 0 && i + 1 < argc) {
            strncpy(config.csv_path, argv[++i], sizeof(config.csv_path) - 1);
            config.csv_path[sizeof(config.csv_path) - 1] = '\0';
        } else if (strcmp(argv[i], "-csv-in") == 0 && i + 1 < argc) {
            config.csv_input_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-csv-out") == 0 && i + 1 < argc) {
            config.csv_output_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-csv-header") == 0) {
            config.csv_has_header = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            config.verbose = 1;
        } else if (strcmp(argv[i], "-?") == 0) {
            print_usage(argv[0]);
            exit(0);
        }
    }
    return config;
}

void print_config(const TrainingConfig *config) {
    printf("training configuration:\n");
    printf("  epochs: %d\n", config->epochs);
    printf("  learning rate: %.6f\n", config->learning_rate);
    printf("  hidden size: %d\n", config->hidden_size);
    printf("  batch size: %d\n", config->batch_size);
    printf("  momentum: %.6f\n", config->momentum);
    printf("  weight decay: %.6f\n", config->weight_decay);
    printf("  shuffle: %s\n", config->shuffle ? "yes" : "no");
    printf("  decay rate: %.6f\n", config->decay_rate);
    printf("  decay steps: %d\n", config->decay_steps);
    printf("  dataset: %s\n", config->dataset_type);
    printf("  dataset size: %d\n", config->dataset_size);
    printf("  train/test split: %.2f\n", config->train_test_split);
    if (config->use_validation_set) {
        printf("  validation ratio: %.2f\n", config->validation_ratio);
    }
    printf("  patience: %d\n", config->patience);
    printf("  hidden activation: %s\n", config->hidden_activation);
    printf("  output activation: %s\n", config->output_activation);
    printf("  loss function: %s\n", config->loss_function);
    printf("  optimizer: %s\n", config->optimizer);
    printf("  gradient check: %s\n", config->gradient_check ? "yes" : "no");
    printf("  verbose: %s\n", config->verbose ? "yes" : "no");
    if (config->seed >= 0) {
        printf("  random seed: %d\n", config->seed);
    } else {
        printf("  random seed: time-based\n");
    }
    if (config->save_path[0] != '\0') { printf("  save model to: %s\n", config->save_path); }
    if (config->load_path[0] != '\0') { printf("  load model from: %s\n", config->load_path); }
    if (config->csv_path[0] != '\0') {
        printf("  csv dataset: %s (in: %d, out: %d, header: %s)\n",
               config->csv_path,
               config->csv_input_size,
               config->csv_output_size,
               config->csv_has_header ? "yes" : "no");
    }
    if (strcmp(config->dataset_type, "circle_enhanced") == 0) {
        printf("  boundary ratio: %.2f\n", config->boundary_ratio);
    }
}

ActivationType get_activation_type(const char *name) {
    if (strcmp(name, "sigmoid") == 0) return ACTIVATION_SIGMOID;
    if (strcmp(name, "relu") == 0) return ACTIVATION_RELU;
    if (strcmp(name, "leaky_relu") == 0) return ACTIVATION_LEAKY_RELU;
    if (strcmp(name, "tanh") == 0) return ACTIVATION_TANH;
    if (strcmp(name, "linear") == 0) return ACTIVATION_LINEAR;
    if (strcmp(name, "softmax") == 0) return ACTIVATION_SOFTMAX;
    return ACTIVATION_RELU; // default
}

LossType get_loss_type(const char *name) {
    if (strcmp(name, "mse") == 0) return LOSS_MSE;
    if (strcmp(name, "bce") == 0 || strcmp(name, "binary_cross_entropy") == 0)
        return LOSS_BINARY_CE;
    if (strcmp(name, "mae") == 0) return LOSS_MAE;
    if (strcmp(name, "cce") == 0 || strcmp(name, "categorical_ce") == 0) return LOSS_CATEGORICAL_CE;
    return LOSS_MSE; // default
}

// predicted class vs target: binary = 0.5 threshold, multiclass = argmax
static void predict_class(const NeuralNetwork *nn,
                          const double *target,
                          int output_size,
                          int is_binary,
                          int *predicted,
                          int *actual) {
    *predicted = 0;
    *actual    = 0;
    if (is_binary) {
        *predicted = (nn->layer[1].a[0] > 0.5) ? 1 : 0;
        *actual    = (target[0] > 0.5) ? 1 : 0;
        return;
    }
    double max_p = nn->layer[1].a[0], max_t = target[0];
    for (int k = 1; k < output_size; k++) {
        if (nn->layer[1].a[k] > max_p) {
            max_p      = nn->layer[1].a[k];
            *predicted = k;
        }
        if (target[k] > max_t) {
            max_t   = target[k];
            *actual = k;
        }
    }
}

static int count_correct(NeuralNetwork *nn, const Dataset *ds, int output_size, int is_binary) {
    int correct = 0;
    for (int i = 0; i < ds->num_samples; i++) {
        forward(nn, ds->inputs[i]);
        int predicted, actual;
        predict_class(nn, ds->targets[i], output_size, is_binary, &predicted, &actual);
        if (predicted == actual) correct++;
    }
    return correct;
}

int main(int argc, char *argv[]) {
    TrainingConfig config = parse_arguments(argc, argv);

    if (config.seed >= 0) {
        srand((unsigned int)config.seed);
    } else {
        srand((unsigned int)time(NULL));
    }

    if (config.verbose) { print_config(&config); }

    Dataset *full_dataset  = NULL;
    Dataset *train_dataset = NULL;
    Dataset *test_dataset  = NULL;

    if (config.csv_path[0] != '\0') {
        full_dataset = load_csv_dataset(
            config.csv_path, config.csv_input_size, config.csv_output_size, config.csv_has_header);
        if (!full_dataset) {
            fprintf(stderr, "error: failed to load CSV dataset from %s\n", config.csv_path);
            return 1;
        }
    } else if (strcmp(config.dataset_type, "sine") == 0) {
        full_dataset = create_sine_wave_dataset(config.dataset_size);
    } else if (strcmp(config.dataset_type, "circle") == 0) {
        full_dataset = create_circle_dataset(config.dataset_size);
    } else if (strcmp(config.dataset_type, "circle_enhanced") == 0) {
        full_dataset = create_circle_dataset_enhanced(config.dataset_size, config.boundary_ratio);
    } else if (strcmp(config.dataset_type, "xor") == 0) {
        full_dataset = create_xor_dataset();
    } else {
        printf("unknown dataset type: %s. using sine wave.\n", config.dataset_type);
        full_dataset = create_sine_wave_dataset(config.dataset_size);
    }

    Dataset *val_dataset = NULL;
    if (config.use_validation_set) {
        double train_ratio = config.train_test_split;
        if (train_ratio + config.validation_ratio >= 1.0) {
            train_ratio = 1.0 - config.validation_ratio - 0.15;
            if (train_ratio <= 0.0) train_ratio = 0.7;
        }
        split_dataset_three_way(full_dataset,
                                train_ratio,
                                config.validation_ratio,
                                &train_dataset,
                                &val_dataset,
                                &test_dataset);
    } else {
        split_dataset(full_dataset, config.train_test_split, &train_dataset, &test_dataset);
    }

    if (config.verbose) {
        printf("\ndataset info:\n");
        printf("  type: %s\n", config.dataset_type);
        printf("  total samples: %d\n", full_dataset->num_samples);
        printf("  training samples: %d\n", train_dataset->num_samples);
        if (val_dataset) printf("  validation samples: %d\n", val_dataset->num_samples);
        printf("  test samples: %d\n", test_dataset->num_samples);
        printf("  input size: %d\n", full_dataset->input_size);
        printf("  output size: %d\n", full_dataset->output_size);
        if (strcmp(config.dataset_type, "circle_enhanced") == 0) {
            printf("  boundary samples ratio: %.2f\n", config.boundary_ratio);
        }
    }

    ActivationType hidden_act = get_activation_type(config.hidden_activation);
    ActivationType output_act = get_activation_type(config.output_activation);
    LossType loss_type        = get_loss_type(config.loss_function);

    NeuralNetwork *nn = NULL;
    if (config.load_path[0] != '\0') {
        nn = load_network(config.load_path);
        if (!nn) {
            fprintf(stderr, "error: failed to load model from %s\n", config.load_path);
            return 1;
        }
        if (config.verbose) { printf("loaded model from %s\n", config.load_path); }
    } else {
        nn = create_network(full_dataset->input_size,
                            config.hidden_size,
                            full_dataset->output_size,
                            hidden_act,
                            output_act,
                            loss_type);
    }
    nn->momentum     = config.momentum;
    nn->weight_decay = config.weight_decay;
    if (strcmp(config.optimizer, "adam") == 0) {
        nn->optimizer = OPTIMIZER_ADAM;
    } else {
        nn->optimizer = OPTIMIZER_SGD;
    }

    if (config.gradient_check) {
        printf("\n=== running gradient check ===\n");
        printf("testing with sample 0\n");
        double max_diff =
            gradient_check(nn, train_dataset->inputs[0], train_dataset->targets[0], 1e-4);

        printf("\ntesting with sample 1\n");
        max_diff = gradient_check(nn, train_dataset->inputs[1], train_dataset->targets[1], 1e-4);

        if (max_diff < 1e-5) {
            printf("\n✓ all gradient checks passed! backpropagation is correct.\n");
        } else {
            printf("\n✗ gradient checks failed! there's an issue with backpropagation.\n");
            printf("continuing training anyway...\n");
        }
        printf("\n");
    }

    if (config.verbose) printf("starting training...\n");

    double best_monitor_error = 1e9;
    int no_improvement_count  = 0;

    for (int epoch = 0; epoch < config.epochs; epoch++) {
        double current_learning_rate = config.learning_rate;
        if (config.decay_rate < 1.0) {
            int decay_cycles      = epoch / config.decay_steps;
            current_learning_rate = config.learning_rate * pow(config.decay_rate, decay_cycles);

            if (config.verbose && epoch % config.decay_steps == 0 && epoch > 0) {
                printf("epoch %d: learning rate decayed to %.6f\n", epoch, current_learning_rate);
            }
        }

        double train_error = train_epoch(nn,
                                         train_dataset->inputs,
                                         train_dataset->targets,
                                         train_dataset->num_samples,
                                         config.batch_size,
                                         current_learning_rate,
                                         config.shuffle);

        double test_error = calculate_loss(
            nn, test_dataset->inputs, test_dataset->targets, test_dataset->num_samples);
        double val_error =
            val_dataset ?
                calculate_loss(
                    nn, val_dataset->inputs, val_dataset->targets, val_dataset->num_samples) :
                0.0;
        double monitor_error = val_dataset ? val_error : test_error;

        if (monitor_error < best_monitor_error) {
            best_monitor_error   = monitor_error;
            no_improvement_count = 0;
        } else {
            no_improvement_count++;
            if (no_improvement_count >= config.patience && config.patience > 0) {
                if (config.verbose) {
                    printf("\nearly stopping at epoch %d (no improvement for %d epochs)\n",
                           epoch,
                           config.patience);
                    printf(
                        "best %s error: %.6f\n", val_dataset ? "val" : "test", best_monitor_error);
                }
                break;
            }
        }

        int log_step = config.epochs / 20;
        if (config.verbose &&
            (log_step == 0 || epoch % log_step == 0 || epoch == config.epochs - 1)) {
            if (val_dataset) {
                printf("epoch %d/%d | train: %.6f | val: %.6f | test: %.6f\n",
                       epoch,
                       config.epochs,
                       train_error,
                       val_error,
                       test_error);
            } else {
                printf("epoch %d/%d | train loss: %.6f | test loss: %.6f\n",
                       epoch,
                       config.epochs,
                       train_error,
                       test_error);
            }
        }

        // print progress bar in non-verbose mode
        int bar_step = config.epochs / 100;
        if (!config.verbose && (bar_step == 0 || epoch % bar_step == 0)) {
            print_progress(epoch, config.epochs, monitor_error);
        }
    }

    if (!config.verbose) {
        printf("\n"); // clear the progress bar line
    }

    // classification problem? (binary threshold or argmax multiclass)
    int is_binary =
        (full_dataset->output_size == 1 &&
         (loss_type == LOSS_BINARY_CE || strcmp(config.output_activation, "sigmoid") == 0));
    int is_multiclass =
        (full_dataset->output_size > 1 &&
         (loss_type == LOSS_CATEGORICAL_CE || strcmp(config.output_activation, "softmax") == 0));

    if (config.verbose) {
        double final_train_error = calculate_loss(
            nn, train_dataset->inputs, train_dataset->targets, train_dataset->num_samples);
        double final_test_error = calculate_loss(
            nn, test_dataset->inputs, test_dataset->targets, test_dataset->num_samples);
        printf("\nfinal results:\n");
        printf("  training loss: %.6f\n", final_train_error);
        if (val_dataset) {
            double final_val_error = calculate_loss(
                nn, val_dataset->inputs, val_dataset->targets, val_dataset->num_samples);
            printf("  validation loss: %.6f\n", final_val_error);
        }
        printf("  test loss: %.6f\n", final_test_error);

        if (is_binary || is_multiclass) {
            int train_correct =
                count_correct(nn, train_dataset, full_dataset->output_size, is_binary);
            int val_correct =
                val_dataset ? count_correct(nn, val_dataset, full_dataset->output_size, is_binary) :
                              0;
            int test_correct =
                count_correct(nn, test_dataset, full_dataset->output_size, is_binary);

            printf("  training accuracy: %.2f%% (%d/%d)\n",
                   100.0 * train_correct / train_dataset->num_samples,
                   train_correct,
                   train_dataset->num_samples);
            if (val_dataset && val_dataset->num_samples > 0) {
                printf("  validation accuracy: %.2f%% (%d/%d)\n",
                       100.0 * val_correct / val_dataset->num_samples,
                       val_correct,
                       val_dataset->num_samples);
            }
            if (test_dataset->num_samples > 0) {
                printf("  test accuracy: %.2f%% (%d/%d)\n",
                       100.0 * test_correct / test_dataset->num_samples,
                       test_correct,
                       test_dataset->num_samples);
            }
        }
    }

    printf("\nsample predictions:\n");
    int num_test_samples_to_show = test_dataset->num_samples < 10 ? test_dataset->num_samples : 10;
    for (int i = 0; i < num_test_samples_to_show; i++) {
        forward(nn, test_dataset->inputs[i]);
        printf("input: [");
        for (int j = 0; j < full_dataset->input_size; j++) {
            printf("%.3f", test_dataset->inputs[i][j]);
            if (j < full_dataset->input_size - 1) printf(", ");
        }
        printf(
            "] -> output: %.4f (expected: %.4f)", nn->layer[1].a[0], test_dataset->targets[i][0]);

        if (is_binary || is_multiclass) {
            int predicted, actual;
            predict_class(nn,
                          test_dataset->targets[i],
                          full_dataset->output_size,
                          is_binary,
                          &predicted,
                          &actual);
            printf(" -> class: %d (expected: %d) %s",
                   predicted,
                   actual,
                   (predicted == actual) ? "✓" : "✗");
        }
        printf("\n");
    }

    if (config.save_path[0] != '\0') {
        if (save_network(nn, config.save_path) == 0) {
            printf("\nmodel saved to %s\n", config.save_path);
        } else {
            fprintf(stderr, "\nerror: failed to save model to %s\n", config.save_path);
        }
    }

    free_network(nn);
    free_dataset(full_dataset);
    free_dataset(train_dataset);
    if (val_dataset) free_dataset(val_dataset);
    free_dataset(test_dataset);

    return 0;
}
