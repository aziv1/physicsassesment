import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from tkinter import Tk
from tkinter.filedialog import askopenfilename
from scipy.interpolate import UnivariateSpline
from cli_thread import cli_loop, set_mass_callback

# === Global Mass 
mass = 0.685

def compute_force():
    df['Force'] = df['Ax'] * mass
    if 'force_line' in globals():
        force_line.set_ydata(df['Force'])
        force_ax.relim()
        force_ax.autoscale_view()
        plt.draw()

def update_mass(new_mass):
    global mass
    try:
        mass = float(new_mass)
        print(f"[INFO] Mass updated to {mass} kg")
        compute_force()
    except ValueError:
        print("[ERROR] Invalid mass value.")

# === Load CSV
Tk().withdraw()
filename = askopenfilename(title="Select your accelerometer CSV file")
if not filename:
    print("No file selected.")
    exit()

df = pd.read_csv(filename)
df.rename(columns={
    'Time(ms)': 'Time',
    'Ax(m/s^2)': 'Ax',
    'Ay(m/s^2)': 'Ay',
    'Az(m/s^2)': 'Az',
    'Gx(deg/s)': 'Gx',
    'Gy(deg/s)': 'Gy',
    'Gz(deg/s)': 'Gz'
}, inplace=True)
df['Time'] /= 1000.0

# === Smooth Ax
time = df['Time'].values
ax = df['Ax'].values
spline = UnivariateSpline(time, ax, s=1e3)
df['AxSmoothed'] = spline(time)

# === Initial Force Calculation
df['Force'] = df['Ax'] * mass

# === Main Plot
fig, (acc_ax, force_ax) = plt.subplots(2, 1, figsize=(12, 10))

# Acceleration subplot
lines = {
    'x': acc_ax.plot(df['Time'], df['Ax'], label='Ax')[0],
    'u': acc_ax.plot(df['Time'], df['AxSmoothed'], label='Ax (Spline Smoothed)', linestyle='--')[0],
    'y': acc_ax.plot(df['Time'], df['Ay'], label='Ay')[0],
    'z': acc_ax.plot(df['Time'], df['Az'], label='Az')[0]
}
acc_ax.set_xlabel('Time (s)')
acc_ax.set_ylabel('Acceleration (m/s²)')
acc_ax.set_title('Acceleration (Raw and Smoothed)')
acc_ax.grid(True)
acc_ax.legend()

# Force subplot
force_line, = force_ax.plot(df['Time'], df['Force'], label='Force (Ax × mass)', color='red')
force_ax.set_xlabel('Time (s)')
force_ax.set_ylabel('Force (N)')
force_ax.set_title('Force vs Time')
force_ax.grid(True)
force_ax.legend()

plt.tight_layout()

def on_key(event):
    key = event.key.lower()
    if key in lines:
        lines[key].set_visible(not lines[key].get_visible())
        plt.draw()

fig.canvas.mpl_connect('key_press_event', on_key)

# === CLI
def handle_cli_result(msg):
    print(f"[CLI RESULT] {msg}")

set_mass_callback(update_mass)  # Allow CLI to call `update_mass`
cli_loop(df, handle_cli_result)

plt.show()
