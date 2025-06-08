def get_values(df, axis, start_time, end_time):
    if axis not in df.columns:
        return f"Axis '{axis}' not found.", []

    try:
        start_time = float(start_time)
        end_time = float(end_time)
    except ValueError:
        return "Start and end times must be numeric.", []

    if start_time > end_time:
        return "Start time must be less than or equal to end time.", []

    # Filter the DataFrame by time range
    mask = (df['Time'] >= start_time) & (df['Time'] <= end_time)
    selected = df.loc[mask, ['Time', axis]]

    if selected.empty:
        return "No data points found in the specified time range.", []

    messages = []
    for i, (time, value) in enumerate(zip(selected['Time'], selected[axis]), start=1):
        messages.append(f"{i}. Time: {time:.3f}s, {axis}: {value:.5f}")

    return None, messages
