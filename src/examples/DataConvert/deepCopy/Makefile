ASCENT_DIR ?= /home/hoody/spack/opt/spack/linux-ubuntu18.04-skylake/gcc-7.5.0/ascent-0.8.0-fgjyh5w3s47yfih3qbetau7dwmlm2mtt

# See $(ASCENT_DIR)/share/ascent/ascent_config.mk for detailed linking info
include $(ASCENT_DIR)/share/ascent/ascent_config.mk

# make 'all' the default target
.PHONY: default
default: all

CXX_FLAGS = -std=c++11
INC_FLAGS = $(ASCENT_INCLUDE_FLAGS)
LNK_FLAGS = $(ASCENT_LINK_RPATH) $(ASCENT_LIB_FLAGS)

deepCopy:
	$(CXX) $(CXX_FLAGS) $(INC_FLAGS) deepCopy.cpp $(LNK_FLAGS) -o deepCopy

all:  deepCopy
	echo "[done building all examples]"

clean:
	rm -f deepCopy