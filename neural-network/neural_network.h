#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum {
    ACTIVATION_SIGMOID,
    ACTIVATION_RELU,
    ACTIVATION_TANH,
    ACTIVATION_LINEAR,
    ACTIVATION_LEAKY_RELU,
    ACTIVATION_SOFTMAX
} ActivationType;

typedef enum {
    LOSS_MSE,
    LOSS_BINARY_CE,      // binary cross-entropy
    LOSS_MAE,            // mean absolute error
    LOSS_CATEGORICAL_CE  // categorical cross-entropy
} LossType;

typedef enum {
    OPTIMIZER_SGD,
    OPTIMIZER_ADAM
} OptimizerType;

// one fully-connected layer: params, gradients, optimizer state, activations
// layer[0] = hidden (rows=hidden_size, cols=input_size)
// layer[1] = output (rows=output_size, cols=hidden_size)
typedef struct {
    int rows;
    int cols;

    double** w;
    double* b;

    double* z; // pre-activation
    double* a; // activation output

    double** dw;
    double* db;

    // optimizer state: momentum buffers (m) and adam second moment (v)
    double** m;
    double* mb;
    double** v;
    double* vb;
} Layer;

// 2-layer network: input -> layer[0] -> layer[1] -> output
typedef struct {
    int input_size;
    int hidden_size;
    int output_size;

    Layer layer[2];

    ActivationType hidden_activation;
    ActivationType output_activation;
    LossType loss_type;

    OptimizerType optimizer;
    int timestep;
    double momentum;

    double weight_decay;
} NeuralNetwork;

double sigmoid(double x);
double relu(double x);
double leaky_relu(double x);
double tanh_activation(double x);
double linear(double x);

double sigmoid_derivative_from_output(double output);
double relu_derivative(double x);
double leaky_relu_derivative(double x);
double tanh_derivative(double x);
double linear_derivative(void);

double activate(double x, ActivationType type);
double activate_derivative(double x, ActivationType type);

double mse_loss(double output, double target);
double binary_cross_entropy_loss(double output, double target);
double mae_loss(double output, double target);
double compute_loss(double output, double target, LossType type);
double compute_loss_derivative(double output, double target, LossType type);

NeuralNetwork* create_network(int input_size, int hidden_size, int output_size,
                              ActivationType hidden_activation, ActivationType output_activation,
                              LossType loss_type);
void forward(NeuralNetwork* nn, double* input);

void backward_accumulate(NeuralNetwork* nn, double* input, double* target);
void update_weights(NeuralNetwork* nn, double learning_rate, int batch_size);
void reset_gradients(NeuralNetwork* nn);

double calculate_loss(NeuralNetwork* nn, double** inputs, double** targets, int num_samples);
void free_network(NeuralNetwork* nn);
double gradient_check(NeuralNetwork* nn, double* input, double* target, double epsilon);
int save_network(const NeuralNetwork* nn, const char* filepath);
NeuralNetwork* load_network(const char* filepath);

double train_epoch(NeuralNetwork* nn, double** inputs, double** targets,
                   int num_samples, int batch_size, double learning_rate, int shuffle);

#endif

