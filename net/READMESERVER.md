#Компиляция
arm-linux-gnueabihf-g++ \
  -static \
  -pthread \
  -o arm_server \
  src/server.cpp \
  src/gpio_control.c \
  -I./library -I./include


#Проверка на пк
qemu-arm-static ./arm_server 192.169.1.100 8080