file(GLOB APP_SOURCES "*.c" "scenes/*.c" "helpers/*.c" "views/*.c")

idf_component_register(
    SRCS ${APP_SOURCES}
    INCLUDE_DIRS "." "scenes" "helpers" "views"
    REQUIRES furi furi_hal gui storage notification json
)
