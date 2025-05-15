import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from tkinter import Tk
from tkinter.filedialog import askopenfilename

# Prompt user to select file
Tk().withdraw()
filename = askopenfilename(title="Select your accelerometer CSV file")
if not filename:
    print("No file selected.")
    exit()

# Load dataset
df = pd.read_csv(filename)

# Rename for easier access
df.rename(columns={
    'Time(ms)': 'Time',
    'Ax(m/s^2)': 'Ax',
    'Ay(m/s^2)': 'Ay',
    'Az(m/s^2)': 'Az',
    'Gx(deg/s)': 'Gx',
    'Gy(deg/s)': 'Gy',
    'Gz(deg/s)': 'Gz'
}, inplace=True)

# Convert time to seconds
df['Time'] = df['Time'] / 1000.0

# Set incline angle (degrees)
theta_deg = 47.79  # <-- Change this to your actual incline angle
theta_rad = np.radians(theta_deg)

# Calculate inclined acceleration A_i = A_y * cos(theta) + A_z * sin(theta)
df['InclinedAccel'] = df['Ay'] * np.cos(theta_rad) + df['Az'] * np.sin(theta_rad)

# Simpson’s Rule integration
def simpsons_rule(x, y):
    n = len(x)
    if n % 2 == 0:
        x, y = x[:-1], y[:-1]
        n -= 1
    h = x[1] - x[0]
    result = y[0] + y[-1]
    for i in range(1, n - 1):
        result += 4 * y[i] if i % 2 else 2 * y[i]
    return result * h / 3

# Calculate the cumulative integral for plotting
integral_values = np.cumsum(df['InclinedAccel'].values) * (df['Time'].values[1] - df['Time'].values[0])
df['CumulativeSimpsonIntegral'] = integral_values

# Print statistics
print("\n=== Dataset Statistics ===")
for col in ['Ax', 'Ay', 'Az', 'InclinedAccel']:
    print(f"\n{col}:")
    print(f"  Mean: {df[col].mean():.3f}")
    print(f"  Min:  {df[col].min():.3f}")
    print(f"  Max:  {df[col].max():.3f}")
print(f"\nSimpson’s Rule Integral of Inclined Acceleration: {integral_values[-1]:.3f} m/s")

# Plotting
fig, ax = plt.subplots(figsize=(12, 6))
lines = {
    'x': ax.plot(df['Time'], df['Ax'], label='Ax')[0],
    'y': ax.plot(df['Time'], df['Ay'], label='Ay')[0],
    'z': ax.plot(df['Time'], df['Az'], label='Az')[0],
    'i': ax.plot(df['Time'], df['InclinedAccel'], label='Inclined Acceleration')[0],
    'u': ax.plot(df['Time'], df['CumulativeSimpsonIntegral'], '--', label='Simpson Integral (Inclined Accel)')[0]
}

# Format plot
ax.set_xlabel('Time (s)')
ax.set_ylabel('Acceleration (m/s²)')
ax.set_title(f'Inclined Acceleration and Simpson\'s Rule Integration (θ = {theta_deg}°)')
ax.grid(True)
ax.legend()
plt.tight_layout()

# Key toggle function (using different keybinds to avoid interference with Matplotlib)
def on_key(event):
    key = event.key.lower()
    if key in lines:
        lines[key].set_visible(not lines[key].get_visible())
        plt.draw()

# Map custom keybinds (x -> Ax, y -> Ay, z -> Az, i -> Inclined Accel, u -> Simpson Integral)
fig.canvas.mpl_connect('key_press_event', on_key)
plt.show()
