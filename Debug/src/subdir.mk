################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/errmap.c \
../src/handle.c \
../src/imap.c \
../src/main.c \
../src/pkg.c \
../src/resp.c \
../src/series.c \
../src/siridb.c \
../src/thread.c \
../src/uvwrap.c 

OBJS += \
./src/errmap.o \
./src/handle.o \
./src/imap.o \
./src/main.o \
./src/pkg.o \
./src/resp.o \
./src/series.o \
./src/siridb.o \
./src/thread.o \
./src/uvwrap.o 

C_DEPS += \
./src/errmap.d \
./src/handle.d \
./src/imap.d \
./src/main.d \
./src/pkg.d \
./src/resp.d \
./src/series.d \
./src/siridb.d \
./src/thread.d \
./src/uvwrap.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I.././include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


