################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../db_loopback.cpp \
../db_mem.cpp \
../db_mysql.cpp \
../db_sqlite3.cpp \
../main.cpp \
../shape_logic.cpp \
../shape_metadata.cpp \
../test.cpp \
../test_record.cpp \
../test_type.cpp 

OBJS += \
./db_loopback.o \
./db_mem.o \
./db_mysql.o \
./db_sqlite3.o \
./main.o \
./shape_logic.o \
./shape_metadata.o \
./test.o \
./test_record.o \
./test_type.o 

CPP_DEPS += \
./db_loopback.d \
./db_mem.d \
./db_mysql.d \
./db_sqlite3.d \
./main.d \
./shape_logic.d \
./shape_metadata.d \
./test.d \
./test_record.d \
./test_type.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


