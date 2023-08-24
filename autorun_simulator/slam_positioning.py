import random
import math
import time 

angle = random.randint(0, 10)

front = 300 / math.cos(math.radians(angle))
rear = 200 / math.cos(math.radians(angle))
left = 100 / math.cos(math.radians(angle))
right = 50 / math.cos(math.radians(angle))

print(angle, front, rear)
min_sum = 10000
while True:
    front = 300 / math.cos(math.radians(angle))
    rear = 200 / math.cos(math.radians(angle))
    left = 100 / math.cos(math.radians(angle))
    right = 50 / math.cos(math.radians(angle))
    sum = front + rear + left + right
    if min_sum > sum:
        angle -= 0.1
        min_sum = sum 
    print(sum, angle)
    time.sleep(0.1)

