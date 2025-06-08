def simpsons_rule(x, y, offset=0.0):
    y = [val + offset for val in y]

    n = len(x)
    if n % 2 == 0:
        x, y = x[:-1], y[:-1]
        n -= 1

    h = (x[-1] - x[0]) / (n - 1)
    result = y[0] + y[-1]
    for i in range(1, n - 1):
        result += 4 * y[i] if i % 2 else 2 * y[i]

    return result * h / 3
