# - Find Gumbo parser!
# Matias Insaurralde <matias@insaurral.de>

FIND_PATH(GUMBO_INCLUDE_DIR gumbo.h )
FIND_LIBRARY(GUMBO_LIBRARY NAMES gumbo )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libgumbo DEFAULT_MSG GUMBO_LIBRARY GUMBO_INCLUDE_DIR)
