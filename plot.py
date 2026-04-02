import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
from dataclasses import dataclass

@dataclass
class Pos:
    x: float; y: float; z: float

# Read simulation parameters
n = int(sys.stdin.readline())
steps = int(sys.stdin.readline())

# Pre-allocate data structure
step_data: list[list[Pos | None]] = [[None] * n for _ in range(steps)]

# Parse input lines (Expected: step,index,x,y,z)
for line in sys.stdin:
    vals = line.strip().split(',')
    if len(vals) < 5: continue
    s, i = map(int, vals[0:2])
    x, y, z = map(float, vals[2:])
    step_data[s][i] = Pos(x, y, z)

# Initialize 3D Plot
fig = plt.figure(figsize=(10, 7))
ax = fig.add_subplot(111, projection='3d')
plt.subplots_adjust(bottom=0.2)

# Extract initial positions
init_xs = [p.x for p in step_data[0] if p is not None]
init_ys = [p.y for p in step_data[0] if p is not None]
init_zs = [p.z for p in step_data[0] if p is not None]

scat = ax.scatter(init_xs, init_ys, init_zs)
ax.set_xlabel('X'), ax.set_ylabel('Y'), ax.set_zlabel('Z')

# Slider Setup
ax_step = plt.axes([0.2, 0.05, 0.6, 0.03])
slider = Slider(ax_step, 'Step', 0, steps - 1, valinit=0, valfmt='%d')

def update(val):
    idx = int(slider.val)
    current_pos = step_data[idx]
    
    # Update 3D scatter coordinates
    xs = [p.x for p in current_pos]
    ys = [p.y for p in current_pos]
    zs = [p.z for p in current_pos]
    
    scat._offsets3d = (xs, ys, zs) # Optimized 3D update
    fig.canvas.draw_idle()

slider.on_changed(update)
plt.show()
