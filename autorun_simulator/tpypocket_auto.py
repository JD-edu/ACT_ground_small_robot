import pygame 
import random 

x = 50
y = 50

def front_sensor(direction):
    if direction == 1:
        return y 
    elif direction == 2:
        return 700 - y
    elif direction == 3:
        return x
    elif direction == 4:
        return 700 - x

def rear_sensor(direction):
    if direction == 1:
        return 800 - y 
    elif direction == 2:
        return y
    elif direction == 3:
        return 800 - x
    elif direction == 4:
        return x

def left_sensor(direction):
    if direction == 1:
        return x 
    elif direction == 2:
        return 800 - x
    elif direction == 3:
        return 800 - y
    elif direction == 4:
        return y

def right_sensor(direction):
    if direction == 1:
        return 800 - x 
    elif direction == 2:
        return x
    elif direction == 3:
        return y
    elif direction == 4:
        return 800-y
    
def object_sensor(dir):
    global x, y,obj_x, obj_y, real_x, real_y
    if dir == right:
        if x == obj_x:
            if obj_y < 700:
                real_y  = obj_y
                return real_y
            else:
                return real_y
    elif dir == down:
        if y == obj_y:
            if obj_x < 700:
                real_x  = obj_x
                return real_y
            else:
                return real_y

    
def scan_space(dir):
    global x, y, obj_x, obj_y
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                break
        if dir == right:
            x += 1
        elif dir == down:
            y += 1 
        value = object_sensor(dir)
        #print(str(value))
        if front_sensor(dir) < 50:
            break
        window.fill((255,255,255))
        pygame.draw.circle(window, (0,0,255), (obj_x, obj_y), 10)
        window.blit(car_img, (x, y))
        pygame.display.update()
    
pygame.init()

'''
direction: respect to pygame axis up = 1  down = 2 left = 3 right = 4
'''
up, down, left, right = 1, 2, 3, 4
direction = up


window = pygame.display.set_mode((800, 800))
car_img = pygame.image.load('./car.png')
car_img = pygame.transform.scale(car_img, (100, 100))
run = True

obj_x = random.randint(300, 500)
obj_y = random.randint(300, 500)

real_x = 0
real_y = 0

print('object point: ', obj_x, obj_y)

scan_space(right)
car_img = pygame.transform.rotate(car_img, -90)
scan_space(down)

print('object point: ', real_x, real_y)

while run:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            run = False

    window.fill((255,255,255))
    window.blit(car_img, (x, y))
    pygame.draw.circle(window, (0,0,255), (obj_x, obj_y), 10)
    pygame.display.update()

pygame.quit()
