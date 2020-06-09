PLARFORM:=HI3516CV500

CXX = arm-himix200-linux-g++
CC  = arm-himix200-linux-gcc
alg=yoloV3
TARGET:=sample_${alg}_$(PLARFORM).bin

CCFLAGS  = -g -Wall -O0  -DDEBUG
CXXFLAGS = -g -Wall -O0  -fpermissive -DDEBUG

LDFLAG = -g -Wl,-z,relro,-z,noexecstack,--disable-new-dtags,-rpath,/lib/:/usr/lib/:/usr/app/lib
EXTERN_LIBS := -lopencv_world
HISI_LIBS :=  -lnnie -live -lmpi  -lsecurec -lupvqe -ldnvqe -lVoiceEngine 
LINKE_BLIS :=  $(HISI_LIBS) $(EXTERN_LIBS) -lrt -lm  -lpthread -ldl -lgomp
 
LDFLAG += -fstack-protector --param ssp-buffer-size=4 -Wfloat-equal -Wshadow -Wformat=2 


#OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

SRCDIR 	 := ./source
SRCS_C   = $(wildcard $(SRCDIR)/*.c)
SRCS_CXX = $(wildcard $(SRCDIR)/*.cpp)

SRC_OBJS_C 		:= $(patsubst %.c,%.o,$(SRCS_C))
SRC_OBJS_CXX 	:= $(patsubst %.cpp,%.o,$(SRCS_CXX))
OBJS = $(SRC_OBJS_C) $(SRC_OBJS_CXX)



HISI_SDK_DIR:=/home/until/work/HISI/Hi3516CV500_SDK_V2.0.0.3/
HISI_SDK_MPP_DIR:=$(HISI_SDK_DIR)/smp/a7_linux/mpp
HISI_SDK_LINUX_big_little_DIR:=$(HISI_SDK_MPP_DIR)/out/linux/big-little
HISI_SDK_LINUX_multi_core_DIR:=$(HISI_SDK_MPP_DIR)/out/linux/multi-core
HISI_SDK_liteos_DIR:=$(HISI_SDK_MPP_DIR)/out/liteos

LINKE_DIR := ./opencv/hisi3516CV500
HISI_DIR :=$(HISI_SDK_MPP_DIR)
#HISI_DIR := $(LINKE_DIR)/hisi
EXTERN_DIR := $(LINKE_DIR)


SAMPLE_RUNTIME_INCLUDE_DIR:= -I$(SRCDIR)
INCDIR  := $(SAMPLE_RUNTIME_INCLUDE_DIR) -I$(HISI_DIR)/include -I$(EXTERN_DIR)/include
LIBS_DIR:= -L$(HISI_DIR)/lib -L$(EXTERN_DIR)/lib

$(warning "CXX="$(CXX))
$(warning "CC="$(CC))
$(warning "CCFLAGS="$(CCFLAGS))
$(warning "CXXFLAGS="$(CXXFLAGS))
$(warning "LDFLAG="$(LDFLAG))

$(warning "INCDIR="$(INCDIR))
$(warning "LIBS_DIR="$(LIBS_DIR))
$(warning "LINKE_BLIS="$(LINKE_BLIS))

$(info "")


$(TARGET):$(SRC_OBJS_C) $(SRC_OBJS_CXX)

	$(CXX) $(SRC_OBJS_C) $(SRC_OBJS_CXX) -o $@ $(INCDIR) $(LIBS_DIR) $(LDFLAG) $(LINKE_BLIS)

$(SRC_OBJS_C):%.o:%.c

	$(CC)  -c  $< -o $@ $(INCDIR) $(CCFLAGS) $(LINKE_BLIS)
	
$(SRC_OBJS_CXX):%.o:%.cpp

	$(CXX) -c  $< -o $@ $(INCDIR) $(CXXFLAGS) $(LINKE_BLIS)

clean:

	rm $(OBJS) $(TARGET)

