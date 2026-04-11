def generate_autoguess_file(num_steps=11):
    lines = []
    lines.append("connection relations")

    # бієкція - знаючи одну змінну, можна знайти іншу
    def rel2(a, b):
        lines.append(f"{a}, {b}")

    # симетричний зв'язок - знаючи будь-які дві знаходимо третю
    def rel3(a, b, c):
        lines.append(f"{a}, {b}, {c}")

    def rel4(a, b, c, d):
        lines.append(f"{a}, {b}, {c}, {d}")

    for t in range(num_steps):
        lines.append(f"# --- Takt {t} ---")

        rel3(f"X_{t}", f"S_{t+15}", f"R1_{t}")
        rel4(f"Z_{t}", f"X_{t}", f"R2_{t}", f"S_{t}")
        rel2(f"R2_{t+1}", f"R1_{t}")
        rel3(f"R1_{t+1}", f"R2_{t}", f"S_{t+13}")
        rel4(f"S_{t+16}", f"S_{t}", f"S_{t+11}", f"S_{t+13}")

    lines.append("known")
    for t in range(num_steps):
        lines.append(f"Z_{t}")

    lines.append("target")
    for i in range(16):
        lines.append(f"S_{i}")
    lines.append("R1_0")
    lines.append("R2_0")

    lines.append("end")
    return "\n".join(lines)


if __name__ == "__main__":
    output = generate_autoguess_file(11)
    with open("strumok_autoguess.txt", "w", encoding="utf-8") as f:
        f.write(output)
    print("strumok_autoguess.txt згенеровано!")