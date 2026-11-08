# Neural Network in C

Modular 2-layer neural network implemented from scratch in pure C.

## Build & Run

```bash
make

# Train on XOR problem
./nn -d xor -v

# Train on circle classification (custom hidden layer & batch size)
./nn -d circle -h 16 -b 64 -v

# Show all flags
./nn -?
```

Presets:
```bash
make run-xor       # XOR gate
make run           # Sine wave function approximation
make run-circle    # Circle boundary classification
```

## Features

- **Architecture:** 2-layer network (input → hidden → output) with configurable hidden layer size.
- **Activations:** Sigmoid, ReLU, tanh, Leaky ReLU, Linear, Softmax.
- **Loss Functions:** Mean Squared Error (MSE), Binary Cross-Entropy (BCE), MAE, Categorical Cross-Entropy (CCE).
- **Optimization:** Mini-batch SGD with momentum or Adam optimizer, learning rate decay, and L2 weight decay.
- **Validation:** 3-way train/val/test split, early stopping on validation loss, full parameter gradient checking (`make test`).
- **Persistence & I/O:** Model serialization/deserialization (`-save`, `-load`), custom CSV dataset loader (`-csv`).

## Datasets

- `xor`: Non-linear logic gate.
- `sine`: 2D function approximation (`sin(x) * cos(y)`).
- `circle` / `circle_enhanced`: 2D point classification (inside vs. outside circle).
- Custom CSV files via `-csv <file> -csv-in <n> -csv-out <n> [-csv-header]`.
