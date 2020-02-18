import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image
import numpy as np

im = np.array(Image.open('samples/im4.pgm'), dtype=np.uint8)

# Create figure and axes
fig,ax = plt.subplots(1)

# Display the image
ax.imshow(im)

# Copy Paste code here:

rect = patches.Rectangle((1,30),20,26,linewidth=1,edgecolor='r',facecolor='none')
ax.add_patch(rect)
rect = patches.Rectangle((36,56),18,22,linewidth=1,edgecolor='r',facecolor='none')
ax.add_patch(rect)
rect = patches.Rectangle((50,5),21,21,linewidth=1,edgecolor='r',facecolor='none')
ax.add_patch(rect)


################################

# Add the patch to the Axes
ax.add_patch(rect)

plt.show()