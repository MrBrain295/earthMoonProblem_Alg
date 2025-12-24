import sys

def read_edges(path):
    edges = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            u, v = map(int, line.split())
            edges.append((u, v))
    return edges


def blow_up(edges):
    blown_edges = []
    for a, b in edges:
        a0 = 2 * a
        a1 = 2 * a + 1
        b0 = 2 * b
        b1 = 2 * b + 1

        # EXACT order expected by biplanarBlowupSAT.py
        blown_edges.append((a0, b0))  # ab
        blown_edges.append((a1, b1))  # a'b'
        blown_edges.append((a1, b0))  # a'b
        blown_edges.append((a0, b1))  # ab'

    return blown_edges


def write_edges(edges, path):
    with open(path, "w") as f:
        for u, v in edges:
            f.write(f"{u} {v}\n")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 blowup_graph.py input.txt output.txt")
        sys.exit(1)

    in_file = sys.argv[1]
    out_file = sys.argv[2]

    edges = read_edges(in_file)
    blown = blow_up(edges)
    write_edges(blown, out_file)

    print(f"Original edges: {len(edges)}")
    print(f"Blown-up edges: {len(blown)}")
