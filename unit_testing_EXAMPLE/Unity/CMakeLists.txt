# Set the source files that make up the library
set(UnitySRC
 unity_internals.h unity.h unity.c 
)
# Set the library type (can be static or dynamic )
set(LIB_TYPE STATIC)

# Create the library
add_library (UnityLib ${LIB_TYPE} ${UnitySRC})
