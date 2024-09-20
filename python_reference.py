# Import necessary libraries
import numpy as np
import matplotlib.pyplot as plt

# Generate a 2D scalar field (e.g., temperature distribution)
def generate_scalar_field(size):
    x = np.linspace(-3.0, 3.0, size)
    y = np.linspace(-3.0, 3.0, size)
    x, y = np.meshgrid(x, y)
    z = np.sin(x**2 + y**2)
    return z

# Apply a simple color map to visualize the scalar field
def visualize_scalar_field(scalar_field):
    plt.imshow(scalar_field, cmap='gray', origin='lower')
    plt.colorbar(label='Scalar Value')
    plt.title('2D Scalar Field Visualization')
    plt.show()

# Set size of scalar field
field_size = 100

# Generate the scalar field
scalar_field = generate_scalar_field(field_size)

# Visualize the scalar field
visualize_scalar_field(scalar_field)