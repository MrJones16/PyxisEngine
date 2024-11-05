set(poly2tri_COMMON_FILES
	common/dll_symbol.h
    common/shapes.cpp
    common/shapes.h
    common/utils.h
	)

set(poly2tri_SWEEP_FILES
	sweep/advancing_front.cpp
	sweep/advancing_front.h
	sweep/cdt.cpp
	sweep/cdt.h
	sweep/sweep_context.cpp
	sweep/sweep_context.h
    sweep/sweep.cpp
    sweep/sweep.h
	)

add_library(poly2tri poly2tri.h ${poly2tri_COMMON_FILES} ${poly2tri_SWEEP_FILES})