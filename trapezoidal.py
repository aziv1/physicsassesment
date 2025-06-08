def trapezoidal_rule(x, y, offset=0.0):
    y = [val + offset for val in y]
    result = 0.0
    for i in range(1, len(x)):
        result += (x[i] - x[i - 1]) * (y[i] + y[i - 1]) / 2
    return result