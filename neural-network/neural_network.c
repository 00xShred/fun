#include "neural_network.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

double sigmoid(double x) {
    if (x >= 0) {
        return 1.0 / (1.0 + exp(-x));
    } else {
        double ex = exp(x);
        return ex / (1.0 + ex);
    }
}

double relu(double x) { return (x > 0) ? x : 0.0; }

double leaky_relu(double x) { return (x > 0) ? x : 0.01 * x; }

double tanh_activation(double x) { return tanh(x); }

double linear(double x) { return x; }

double sigmoid_derivative_from_output(double output) { return output * (1.0 - output); }

double relu_derivative(double x) { return (x > 0) ? 1.0 : 0.0; }

double leaky_relu_derivative(double x) { return (x > 0) ? 1.0 : 0.01; }

double tanh_derivative(double x) {
    double th = tanh(x);
    return 1.0 - th * th;
}

double linear_derivative(void) { return 1.0; }

double activate(double x, ActivationType type) {
    switch (type) {
        case ACTIVATION_SIGMOID: return sigmoid(x);
        case ACTIVATION_RELU: return relu(x);
        case ACTIVATION_LEAKY_RELU: return leaky_relu(x);
        case ACTIVATION_TANH: return tanh_activation(x);
        case ACTIVATION_LINEAR: return linear(x);
        default: return relu(x);
    }
}

double activate_derivative(double x, ActivationType type) {
    switch (type) {
        case ACTIVATION_SIGMOID:
            return sigmoid_derivative_from_output(activate(x, ACTIVATION_SIGMOID));
        case ACTIVATION_RELU: return relu_derivative(x);
        case ACTIVATION_LEAKY_RELU: return leaky_relu_derivative(x);
        case ACTIVATION_TANH: return tanh_derivative(x);
        case ACTIVATION_LINEAR: return linear_derivative();
        default: return relu_derivative(x);
    }
}

double mse_loss(double output, double target) {
    double error = target - output;
    return error * error;
}

double binary_cross_entropy_loss(double output, double target) {
    // add epsilon to avoid log(0)
    double epsilon = 1e-12;
    output         = fmax(epsilon, fmin(1.0 - epsilon, output));
    return -(target * log(output) + (1.0 - target) * log(1.0 - output));
}

double mae_loss(double output, double target) { return fabs(target - output); }

double categorical_cross_entropy_loss(double output, double target) {
    double epsilon = 1e-12;
    output         = fmax(epsilon, fmin(1.0 - epsilon, output));
    return -target * log(output);
}

double compute_loss(double output, double target, LossType type) {
    switch (type) {
        case LOSS_MSE: return mse_loss(output, target);
        case LOSS_BINARY_CE: return binary_cross_entropy_loss(output, target);
        case LOSS_MAE: return mae_loss(output, target);
        case LOSS_CATEGORICAL_CE: return categorical_cross_entropy_loss(output, target);
        default: return mse_loss(output, target);
    }
}

double compute_loss_derivative(double output, double target, LossType type) {
    double epsilon = 1e-12;
    output         = fmax(epsilon, fmin(1.0 - epsilon, output));

    switch (type) {
        case LOSS_MSE: return 2.0 * (output - target); // dL/doutput for mse
        case LOSS_BINARY_CE:
            // for bce: dL/doutput = (output - target) / (output * (1 - output))
            return (output - target) / (output * (1 - output));
        case LOSS_MAE: return (output > target) ? 1.0 : -1.0;
        case LOSS_CATEGORICAL_CE: return -target / output;
        default: return 2.0 * (output - target);
    }
}

static void alloc_layer(Layer *l, int rows, int cols) {
    l->rows = rows;
    l->cols = cols;
    l->w    = (double **)malloc(rows * sizeof(double *));
    l->b    = (double *)calloc(rows, sizeof(double));
    l->dw   = (double **)malloc(rows * sizeof(double *));
    l->db   = (double *)calloc(rows, sizeof(double));
    l->m    = (double **)malloc(rows * sizeof(double *));
    l->mb   = (double *)calloc(rows, sizeof(double));
    l->v    = (double **)malloc(rows * sizeof(double *));
    l->vb   = (double *)calloc(rows, sizeof(double));
    l->z    = (double *)calloc(rows, sizeof(double));
    l->a    = (double *)calloc(rows, sizeof(double));

    for (int i = 0; i < rows; i++) {
        l->w[i]  = (double *)malloc(cols * sizeof(double));
        l->dw[i] = (double *)calloc(cols, sizeof(double));
        l->m[i]  = (double *)calloc(cols, sizeof(double));
        l->v[i]  = (double *)calloc(cols, sizeof(double));
    }

    if (!l->w || !l->b || !l->dw || !l->db || !l->m || !l->mb || !l->v || !l->vb || !l->z ||
        !l->a) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < rows; i++) {
        if (!l->w[i] || !l->dw[i] || !l->m[i] || !l->v[i]) {
            perror("memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }
}

// he initialization for relu-family activations, xavier otherwise
static void init_layer_weights(Layer *l, ActivationType act) {
    for (int i = 0; i < l->rows; i++) {
        for (int j = 0; j < l->cols; j++) {
            double limit;
            if (act == ACTIVATION_RELU || act == ACTIVATION_LEAKY_RELU)
                limit = sqrt(2.0 / l->cols); // he initialization
            else limit = sqrt(2.0 / (l->cols + l->rows)); // xavier initialization
            l->w[i][j] = ((double)rand() / RAND_MAX) * 2 * limit - limit;
        }
    }
}

NeuralNetwork *create_network(int input_size,
                              int hidden_size,
                              int output_size,
                              ActivationType hidden_activation,
                              ActivationType output_activation,
                              LossType loss_type) {
    NeuralNetwork *nn = (NeuralNetwork *)malloc(sizeof(NeuralNetwork));
    if (!nn) {
        perror("failed to allocate neural network");
        exit(EXIT_FAILURE);
    }

    nn->input_size        = input_size;
    nn->hidden_size       = hidden_size;
    nn->output_size       = output_size;
    nn->hidden_activation = hidden_activation;
    nn->output_activation = output_activation;
    nn->loss_type         = loss_type;
    nn->weight_decay      = 0.0;
    nn->optimizer         = OPTIMIZER_SGD;
    nn->timestep          = 0;
    nn->momentum          = 0.0;

    alloc_layer(&nn->layer[0], hidden_size, input_size);
    alloc_layer(&nn->layer[1], output_size, hidden_size);

    // keep rand() consumption order: hidden weights first, then output weights
    init_layer_weights(&nn->layer[0], hidden_activation);
    init_layer_weights(&nn->layer[1], output_activation);

    return nn;
}

void forward(NeuralNetwork *nn, double *input) {
    Layer *h = &nn->layer[0];
    Layer *o = &nn->layer[1];

    for (int i = 0; i < h->rows; i++) {
        h->z[i] = h->b[i];
        for (int j = 0; j < h->cols; j++) { h->z[i] += input[j] * h->w[i][j]; }
        h->a[i] = activate(h->z[i], nn->hidden_activation);
    }

    for (int i = 0; i < o->rows; i++) {
        o->z[i] = o->b[i];
        for (int j = 0; j < o->cols; j++) { o->z[i] += h->a[j] * o->w[i][j]; }
    }

    if (nn->output_activation == ACTIVATION_SOFTMAX) {
        double max_z = o->z[0];
        for (int i = 1; i < o->rows; i++) {
            if (o->z[i] > max_z) max_z = o->z[i];
        }
        double sum = 0.0;
        for (int i = 0; i < o->rows; i++) {
            o->a[i] = exp(o->z[i] - max_z);
            sum += o->a[i];
        }
        for (int i = 0; i < o->rows; i++) { o->a[i] /= sum; }
    } else {
        for (int i = 0; i < o->rows; i++) { o->a[i] = activate(o->z[i], nn->output_activation); }
    }
}

void backward_accumulate(NeuralNetwork *nn, double *input, double *target) {
    Layer *h = &nn->layer[0];
    Layer *o = &nn->layer[1];

    // stack-allocated output deltas; avoids per-sample heap churn
    double delta_output[o->rows];

    if (nn->output_activation == ACTIVATION_SOFTMAX) {
        if (nn->loss_type == LOSS_CATEGORICAL_CE) {
            for (int i = 0; i < o->rows; i++) { delta_output[i] = o->a[i] - target[i]; }
        } else {
            double dot = 0.0;
            double errors[o->rows];
            for (int i = 0; i < o->rows; i++) {
                errors[i] = compute_loss_derivative(o->a[i], target[i], nn->loss_type);
                dot += errors[i] * o->a[i];
            }
            for (int i = 0; i < o->rows; i++) { delta_output[i] = o->a[i] * (errors[i] - dot); }
        }
    } else {
        for (int i = 0; i < o->rows; i++) {
            double output_error = compute_loss_derivative(o->a[i], target[i], nn->loss_type);
            delta_output[i] = output_error * activate_derivative(o->z[i], nn->output_activation);
        }
    }

    for (int i = 0; i < o->rows; i++) {
        for (int j = 0; j < o->cols; j++) { o->dw[i][j] += delta_output[i] * h->a[j]; }
        o->db[i] += delta_output[i];
    }

    for (int i = 0; i < h->rows; i++) {
        double hidden_error = 0;
        for (int j = 0; j < o->rows; j++) { hidden_error += delta_output[j] * o->w[j][i]; }

        double delta_hidden = hidden_error * activate_derivative(h->z[i], nn->hidden_activation);

        for (int j = 0; j < h->cols; j++) { h->dw[i][j] += delta_hidden * input[j]; }
        h->db[i] += delta_hidden;
    }
}

// per-parameter adam update (m/v = first/second moment buffers)
static void adam_update(
    double *p, double g, double *m, double *v, double lr, double decay, double fix1, double fix2) {
    const double beta1 = 0.9, beta2 = 0.999, eps = 1e-8;
    *m            = beta1 * *m + (1.0 - beta1) * g;
    *v            = beta2 * *v + (1.0 - beta2) * g * g;
    double update = -lr * (*m / fix1) / (sqrt(*v / fix2) + eps);
    update -= lr * decay * *p;
    *p += update;
}

// per-parameter sgd + momentum update (prev = momentum buffer)
static void
sgd_update(double *p, double g, double *prev, double lr, double momentum, double decay) {
    double update = -lr * g + momentum * *prev;
    update -= lr * decay * *p;
    *p += update;
    *prev = update;
}

void update_weights(NeuralNetwork *nn, double learning_rate, int batch_size) {
    if (nn->optimizer == OPTIMIZER_ADAM) {
        nn->timestep++;
        double fix1 = 1.0 - pow(0.9, nn->timestep);
        double fix2 = 1.0 - pow(0.999, nn->timestep);

        for (int l = 0; l < 2; l++) {
            Layer *ly = &nn->layer[l];
            for (int i = 0; i < ly->rows; i++) {
                for (int j = 0; j < ly->cols; j++)
                    adam_update(&ly->w[i][j],
                                ly->dw[i][j] / batch_size,
                                &ly->m[i][j],
                                &ly->v[i][j],
                                learning_rate,
                                nn->weight_decay,
                                fix1,
                                fix2);
                adam_update(&ly->b[i],
                            ly->db[i] / batch_size,
                            &ly->mb[i],
                            &ly->vb[i],
                            learning_rate,
                            0.0,
                            fix1,
                            fix2);
            }
        }
        return;
    }

    // default: sgd with momentum; weight decay on weights only
    for (int l = 0; l < 2; l++) {
        Layer *ly = &nn->layer[l];
        for (int i = 0; i < ly->rows; i++) {
            for (int j = 0; j < ly->cols; j++)
                sgd_update(&ly->w[i][j],
                           ly->dw[i][j] / batch_size,
                           &ly->m[i][j],
                           learning_rate,
                           nn->momentum,
                           nn->weight_decay);
            sgd_update(
                &ly->b[i], ly->db[i] / batch_size, &ly->mb[i], learning_rate, nn->momentum, 0.0);
        }
    }
}

void reset_gradients(NeuralNetwork *nn) {
    for (int l = 0; l < 2; l++) {
        Layer *ly = &nn->layer[l];
        for (int i = 0; i < ly->rows; i++) {
            for (int j = 0; j < ly->cols; j++) { ly->dw[i][j] = 0.0; }
            ly->db[i] = 0.0;
        }
    }
}

double train_epoch(NeuralNetwork *nn,
                   double **inputs,
                   double **targets,
                   int num_samples,
                   int batch_size,
                   double learning_rate,
                   int shuffle) {
    if (shuffle) {
        // fisher-yates shuffle
        for (int i = num_samples - 1; i > 0; i--) {
            int j = rand() % (i + 1);

            double *temp_input = inputs[i];
            inputs[i]          = inputs[j];
            inputs[j]          = temp_input;

            double *temp_target = targets[i];
            targets[i]          = targets[j];
            targets[j]          = temp_target;
        }
    }

    double total_loss = 0;

    int batch_start = 0;
    while (batch_start < num_samples) {
        // calculate actual batch size (last batch might be smaller)
        int actual_batch_size = batch_size;
        if (batch_start + actual_batch_size > num_samples) {
            actual_batch_size = num_samples - batch_start;
        }

        reset_gradients(nn);

        for (int i = 0; i < actual_batch_size; i++) {
            int sample_idx = batch_start + i;
            forward(nn, inputs[sample_idx]);
            backward_accumulate(nn, inputs[sample_idx], targets[sample_idx]);

            for (int j = 0; j < nn->output_size; j++) {
                total_loss +=
                    compute_loss(nn->layer[1].a[j], targets[sample_idx][j], nn->loss_type);
            }
        }

        update_weights(nn, learning_rate, actual_batch_size);

        batch_start += actual_batch_size;
    }

    return total_loss / (num_samples * nn->output_size);
}

double calculate_loss(NeuralNetwork *nn, double **inputs, double **targets, int num_samples) {
    if (num_samples <= 0) return 0.0;
    double total_loss = 0;
    for (int s = 0; s < num_samples; s++) {
        forward(nn, inputs[s]);
        for (int i = 0; i < nn->output_size; i++) {
            total_loss += compute_loss(nn->layer[1].a[i], targets[s][i], nn->loss_type);
        }
    }
    return total_loss / (num_samples * nn->output_size);
}

void free_network(NeuralNetwork *nn) {
    if (!nn) return;

    for (int l = 0; l < 2; l++) {
        Layer *ly = &nn->layer[l];
        if (!ly->w) continue;
        for (int i = 0; i < ly->rows; i++) {
            free(ly->w[i]);
            free(ly->dw[i]);
            free(ly->m[i]);
            free(ly->v[i]);
        }
        free(ly->w);
        free(ly->dw);
        free(ly->m);
        free(ly->v);
        free(ly->b);
        free(ly->db);
        free(ly->mb);
        free(ly->vb);
        free(ly->z);
        free(ly->a);
    }
    free(nn);
}

int save_network(const NeuralNetwork *nn, const char *filepath) {
    if (!nn || !filepath) return -1;
    FILE *fp = fopen(filepath, "w");
    if (!fp) return -1;

    fprintf(fp,
            "%d %d %d %d %d %d\n",
            nn->input_size,
            nn->hidden_size,
            nn->output_size,
            (int)nn->hidden_activation,
            (int)nn->output_activation,
            (int)nn->loss_type);

    for (int l = 0; l < 2; l++) {
        const Layer *ly = &nn->layer[l];
        for (int i = 0; i < ly->rows; i++) {
            for (int j = 0; j < ly->cols; j++) { fprintf(fp, "%.17g ", ly->w[i][j]); }
            fprintf(fp, "\n");
        }
        for (int i = 0; i < ly->rows; i++) { fprintf(fp, "%.17g ", ly->b[i]); }
        fprintf(fp, "\n");
    }

    fclose(fp);
    return 0;
}

NeuralNetwork *load_network(const char *filepath) {
    if (!filepath) return NULL;
    FILE *fp = fopen(filepath, "r");
    if (!fp) return NULL;

    int in_sz, hid_sz, out_sz, ha, oa, lt;
    if (fscanf(fp, "%d %d %d %d %d %d", &in_sz, &hid_sz, &out_sz, &ha, &oa, &lt) != 6) {
        fclose(fp);
        return NULL;
    }

    NeuralNetwork *nn =
        create_network(in_sz, hid_sz, out_sz, (ActivationType)ha, (ActivationType)oa, (LossType)lt);

    for (int l = 0; l < 2; l++) {
        Layer *ly = &nn->layer[l];
        for (int i = 0; i < ly->rows; i++) {
            for (int j = 0; j < ly->cols; j++) {
                if (fscanf(fp, "%lf", &ly->w[i][j]) != 1) {
                    free_network(nn);
                    fclose(fp);
                    return NULL;
                }
            }
        }
        for (int i = 0; i < ly->rows; i++) {
            if (fscanf(fp, "%lf", &ly->b[i]) != 1) {
                free_network(nn);
                fclose(fp);
                return NULL;
            }
        }
    }

    fclose(fp);
    return nn;
}

// helper to perturb one parameter and check numerical vs analytical gradient
static void check_param(NeuralNetwork *nn,
                        double *param,
                        double analytical_grad,
                        double *input,
                        double *target,
                        double epsilon,
                        const char *name,
                        double tolerance,
                        double *max_diff) {
    double original = *param;

    *param = original + epsilon;
    forward(nn, input);
    double loss_plus = 0;
    for (int k = 0; k < nn->output_size; k++) {
        loss_plus += compute_loss(nn->layer[1].a[k], target[k], nn->loss_type);
    }

    *param = original - epsilon;
    forward(nn, input);
    double loss_minus = 0;
    for (int k = 0; k < nn->output_size; k++) {
        loss_minus += compute_loss(nn->layer[1].a[k], target[k], nn->loss_type);
    }

    *param = original;

    double numerical_grad = (loss_plus - loss_minus) / (2.0 * epsilon);
    double diff           = fabs(numerical_grad - analytical_grad);
    double avg            = (fabs(numerical_grad) + fabs(analytical_grad)) / 2.0;
    double rel_diff       = (avg > 1e-10) ? diff / avg : 0.0;

    if (rel_diff > *max_diff) { *max_diff = rel_diff; }

    if (rel_diff > tolerance) {
        printf("  %s: num=%.6e, ana=%.6e, diff=%.6e, rel=%.6e\n",
               name,
               numerical_grad,
               analytical_grad,
               diff,
               rel_diff);
    }
}

double gradient_check(NeuralNetwork *nn, double *input, double *target, double epsilon) {
    double max_diff  = 0.0;
    double tolerance = 1e-5;

    printf("\n=== gradient checking ===\n");

    // disable momentum for clean gradient computation
    double saved_momentum = nn->momentum;
    nn->momentum          = 0.0;

    reset_gradients(nn);
    forward(nn, input);
    backward_accumulate(nn, input, target);

    printf("checking w1, b1, w2, b2 gradients...\n");
    for (int l = 0; l < 2; l++) {
        Layer *ly = &nn->layer[l];
        for (int i = 0; i < ly->rows; i++) {
            for (int j = 0; j < ly->cols; j++) {
                char name[32];
                snprintf(name, sizeof(name), "w%d[%d][%d]", l + 1, i, j);
                check_param(nn,
                            &ly->w[i][j],
                            ly->dw[i][j],
                            input,
                            target,
                            epsilon,
                            name,
                            tolerance,
                            &max_diff);
            }
            char b_name[32];
            snprintf(b_name, sizeof(b_name), "b%d[%d]", l + 1, i);
            check_param(
                nn, &ly->b[i], ly->db[i], input, target, epsilon, b_name, tolerance, &max_diff);
        }
    }

    nn->momentum = saved_momentum;

    printf("maximum relative difference: %.2e\n", max_diff);
    if (max_diff < tolerance) {
        printf("✓ gradient check passed!\n");
    } else {
        printf("✗ gradient check failed!\n");
    }

    return max_diff;
}
