# Include from git submodule
idf_component_register(SRCS "src/NVSLog.cpp"  "src/NVSResult.cpp"  "src/NVSStringValue.cpp"  "src/NVSUtils.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES driver nvs_flash)
