# The STD EFFS common utility headers are here
# NBINCLUDE += -I"$(NNDK_ROOT)/examples/_common/EFFS/FAT"

# Source files
CPP_SRC		+= \
				src/FileSystemUtils.cpp \
				src/effs_time.cpp \
				src/ftp_f.cpp \
				src/ramdrv_mcf.cpp

COMMON_FILES := \
				src/FileSystemUtils.cpp \
				src/effs_time.cpp \
				src/ftp_f.cpp \
				src/ramdrv_mcf.cpp \
				src/http_f.h \
				src/FileSystemUtils.h \
				src/effs_time.h \
				src/ftp_f.h \
				src/cardtype.h \

all: $(COMMON_FILES)

debug: $(COMMON_FILES)

$(COMMON_FILES):
	cp $(NNDK_ROOT)/examples/_common/EFFS/FAT/$@ $@

clean-common:
	rm -f $(COMMON_FILES)
