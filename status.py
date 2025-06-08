import numpy as np

def get_status(df, axis):
    if axis not in df.columns:
        return f"Axis '{axis}' not found.", []

    values = df[axis].values
    times = df['Time'].values

    max_idx = np.argmax(values)
    min_idx = np.argmin(values)
    average = np.mean(values)

    max_val = values[max_idx]
    max_time = times[max_idx]
    min_val = values[min_idx]
    min_time = times[min_idx]

    messages = [
        f"Status of {axis}:",
        f"  Max: {max_val:.5f} at {max_time:.3f}s",
        f"  Min: {min_val:.5f} at {min_time:.3f}s",
        f"  Avg: {average:.5f}"
    ]
    return None, messages