# Common flags
CC=gcc
CFLAGS=-O0 -ggdb

# Utils
MKDIR = mkdir -p
RM_FILE=rm -f


# --- SSW UTILS ---------------------------------------------------------------------------
SSW_UTILS_DIR=ssw_utils
SSW_UTILS_COMMON_DIR=$(SSW_UTILS_DIR)/ssw_common
SSW_UTILS_LOGGER_DIR=$(SSW_UTILS_DIR)/ssw_logger

# SSW UTILS - common
SSW_UTILS_COMMON_INC=$(SSW_UTILS_COMMON_DIR)/ssw_status.h $(SSW_UTILS_COMMON_DIR)/ssw_types.h
SSW_UTILS_COMMON_SRC=$(SSW_UTILS_COMMON_DIR)/ssw_status.c

# SSW UTILS - logger
SSW_UTILS_LOGGER_INC=$(SSW_UTILS_LOGGER_DIR)/ssw_logger.h
SSW_UTILS_LOGGER_SRC=$(SSW_UTILS_LOGGER_DIR)/ssw_logger.c

# SSW UTILS
SSW_UTILS_INC=$(SSW_UTILS_COMMON_INC) $(SSW_UTILS_LOGGER_INC)
SSW_UTILS_SRC=$(SSW_UTILS_COMMON_SRC) $(SSW_UTILS_LOGGER_SRC)

# --- SSW HWEMU ---------------------------------------------------------------------------
SSW_HWEMU_BIN_DIR=ssw_hwemu/bin
SSW_HWEMU_SRC_DIR=ssw_hwemu/src
SSW_HWEMU_INC=$(SSW_UTILS_INC)
SSW_HWEMU_SRC=$(SSW_HWEMU_SRC_DIR)/ssw_hwemu.c $(SSW_UTILS_SRC)

SSW_HWEMU_TARGET=$(SSW_HWEMU_BIN_DIR)/ssw_hwemu

$(SSW_HWEMU_TARGET): $(SSW_HWEMU_SRC) $(SSW_HWEMU_INC)
	$(MKDIR) $(SSW_HWEMU_BIN_DIR)
	$(CC) $(CFLAGS) $(SSW_HWEMU_SRC) $(SSW_HWEMU_INC) -o $(SSW_HWEMU_TARGET)

$(SSW_HWEMU_CLEAN):
	$(RM_FILE) $(SSW_HWEMU_TARGET)

all: $(SSW_HWEMU_TARGET)

clean:
	$(RM_FILE) $(SSW_HWEMU_TARGET)