import threading
import sys
import numpy as np

mass_callback = None

def set_mass_callback(callback):
    global mass_callback
    mass_callback = callback

# Import your integration and status methods
from simpsons import simpsons_rule
from trapezoidal import trapezoidal_rule
from status import get_status
from values import get_values

def cli_loop(df, result_callback):
    def loop():
        while True:
            try:
                raw = input(">>> ").strip()
                if not raw:
                    continue
                parts = raw.split()
                cmd = parts[0].upper()

                if cmd == 'INTEGRATE':
                    if len(parts) < 5:
                        print("Usage: INTEGRATE <METHOD> <AXIS> <START_TIME> <END_TIME> [OFFSET]")
                        print("Example: INTEGRATE SIMPSONS AxSmoothed 0 5 0.1")
                        continue

                    method = parts[1].upper()
                    axis = parts[2]
                    try:
                        start_time = float(parts[3])
                        end_time = float(parts[4])
                        offset = float(parts[5]) if len(parts) >= 6 else 0.0
                    except (IndexError, ValueError):
                        print("Please specify valid start and end times (and optional offset as float).")
                        continue

                    if axis not in df.columns:
                        print(f"Axis '{axis}' not found.")
                        continue

                    # Filter by time range
                    mask = (df['Time'] >= start_time) & (df['Time'] <= end_time)
                    times = df.loc[mask, 'Time'].values
                    values = df.loc[mask, axis].values

                    if len(times) < 2:
                        print("Not enough data points in the specified time range.")
                        continue

                    if method == 'SIMPSONS':
                        integral = simpsons_rule(times, values, offset)
                    elif method == 'TRAPEZOIDAL':
                        integral = trapezoidal_rule(times, values, offset)
                    else:
                        print(f"Unknown integration method '{method}'. Use SIMPSONS or TRAPEZOIDAL.")
                        continue

                    result_callback(f"Integral of {axis} from {start_time}s to {end_time}s using {method} with offset {offset}: {integral:.5f}")

                elif cmd == 'STATUS':
                    if len(parts) != 2:
                        print("Usage: STATUS <AXIS>")
                        continue

                    axis = parts[1]
                    error, messages = get_status(df, axis)

                    if error:
                        print(error)
                        continue

                    for msg in messages:
                        result_callback(msg)

                elif cmd == 'VALUES':
                    if len(parts) != 4:
                        print("Usage: VALUES <AXIS> <START_TIME> <END_TIME>")
                        continue

                    axis = parts[1]
                    start_time = parts[2]
                    end_time = parts[3]

                    error, messages = get_values(df, axis, start_time, end_time)

                    if error:
                        print(error)
                        continue

                    for msg in messages:
                        result_callback(msg)

                elif cmd == 'MASS':
                    if len(parts) != 2:
                        print("Usage: MASS <VALUE>")
                        continue
                    if mass_callback:
                        mass_callback(parts[1])
                    else:
                        print("Mass update function not available.")

                elif cmd == 'HELP':
                    print("Commands:")
                    print(" INTEGRATE <METHOD> <AXIS> <START_TIME> <END_TIME>")
                    print(" Example: INTEGRATE SIMPSONS AxSmoothed 0 5")
                    print(" STATUS <AXIS>")
                    print(" Example: STATUS AxSmoothed")
                    print(" Available axes:", ", ".join(df.columns))
                    print(" Methods: SIMPSONS, TRAPEZOIDAL")

                elif cmd in ('EXIT', 'QUIT'):
                    print("Exiting CLI...")
                    break

                else:
                    print(f"Unknown command '{cmd}'. Type HELP for commands.")

            except EOFError:
                break
            except Exception as e:
                print(f"Error: {e}")

    thread = threading.Thread(target=loop, daemon=True)
    thread.start()
