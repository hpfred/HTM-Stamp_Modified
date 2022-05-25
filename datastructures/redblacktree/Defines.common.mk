# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================


PROG := redblacktree

SRCS += $(LIB)/mt19937ar.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \

CXXSRCS := redblacktree.cpp \
        rbtree-htm.cpp \


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================