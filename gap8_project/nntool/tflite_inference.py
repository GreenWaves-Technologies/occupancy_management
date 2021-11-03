import numpy as np
import tensorflow as tf
from PIL import Image

# Load the TFLite model and allocate tensors.
interpreter = tf.lite.Interpreter(model_path="../model/lynred.tflite")
interpreter.allocate_tensors()

# Get input and output tensors.
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Test the model on random input data.
input_shape = input_details[0]['shape']
print("Input Shape Expected: %s" % input_shape)

image = Image.open('../samples/im1.png')

input_data = np.float32(image) #np.array(np.random.random_sample(input_shape), dtype=np.float32)
input_data =input_data.reshape((1,80,80,1))
interpreter.set_tensor(input_details[0]['index'], input_data)

interpreter.invoke()

# The function `get_tensor()` returns a copy of the tensor data.
# Use `tensor()` in order to get a pointer to the tensor.

classes_out_1 = interpreter.get_tensor(output_details[0]['index'])
classes_out_2 = interpreter.get_tensor(output_details[1]['index'])
classes_out_3 = interpreter.get_tensor(output_details[2]['index'])
classes_out_4 = interpreter.get_tensor(output_details[3]['index'])

boxes_out_1 = interpreter.get_tensor(output_details[4]['index'])
boxes_out_2 = interpreter.get_tensor(output_details[5]['index'])
boxes_out_3 = interpreter.get_tensor(output_details[6]['index'])
boxes_out_4 = interpreter.get_tensor(output_details[7]['index'])

