if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico_w) 
cmake_minimum_required(VERSION 3.13)

# Configurações de Compilação
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o SDK da Pico
include(pico_sdk_import.cmake)
project(Projeto_Finale C CXX ASM)
pico_sdk_init()

add_compile_definitions(LWIP_HTTPC_HAVE_FILE_IO=0)
add_compile_definitions(LWIP_HTTPC_HAVE_FILE_IO=0)
add_compile_definitions(HTTPC_DEBUG=1)

# Define o executável 
add_executable(Projeto_Finale 
    src/Projeto_Finale.c 
    src/ssd1306.c
    )

# Habilita o printf pelo cabo USB
pico_enable_stdio_usb(Projeto_Finale 1)
pico_enable_stdio_uart(Projeto_Finale 0)

# Gera o header do PIO
pico_generate_pio_header(Projeto_Finale ${CMAKE_CURRENT_LIST_DIR}/src/blink.pio)

# Vincula as bibliotecas
target_link_libraries(Projeto_Finale 
    pico_stdlib 
    hardware_i2c 
    hardware_adc
    hardware_pio
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
)

# Onde procurar os arquivos .h
target_include_directories(Projeto_Finale PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

# Gera o arquivo .uf2 para a placa
pico_add_extra_outputs(Projeto_Finale)
