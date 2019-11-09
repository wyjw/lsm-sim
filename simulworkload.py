with open(filename) as f:
    lines = f.readlines()
desired_lines = lines[start:end:step]
