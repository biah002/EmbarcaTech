#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/http_client.h"
#include <string.h>

// Definições de hardware e credenciais de rede
#define WIFI_SSID "Biah" // Nome do Wifi
#define WIFI_PASSWORD "senha12345678" // Senha do Wifi

// Pinos dos componentes (LEDs, Buzzer, Joystick e Botões)
#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11
#define BUZZER_A 10
#define BUZZER_B 21
#define JOY_X 26
#define JOY_Y 27
#define BOTAO_A 5
#define BOTAO_B 6

int contador_intrusos = 0;
uint32_t ultimo_envio_ms = 0;
#define INTERVALO_ENVIO_MS 20000
const char* API_KEY = "CZVYPDU9ZXMGC4G9";

static bool envio_pendente = false;

err_t http_recv_cb(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err) {
    if (p != NULL) {
        altcp_recved(conn, p->tot_len);
        pbuf_free(p);
    }
    return ERR_OK;
}

// Callback executado quando o ThingSpeak responde à requisição
void http_result_cb(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) {
    envio_pendente = false;
    if (httpc_result == HTTPC_RESULT_OK) {
        printf("ThingSpeak: enviado! HTTP %lu\n", (unsigned long)srv_res);
    } else {
        printf("ThingSpeak: falha (resultado=%d)\n", (int)httpc_result);
    }
}

// Monta a URL e inicia a requisição HTTP GET para a API do ThingSpeak
void enviar_alertas_thingspeak(int alertas) {
    if (envio_pendente) return; // já tem envio em andamento, ignora

    static char uri[128];
    // Envia o contador de intrusos para o Field 2 da API
    snprintf(uri, sizeof(uri), "/update?api_key=%s&field2=%d", API_KEY, alertas);

    static httpc_connection_t settings;
    memset(&settings, 0, sizeof(settings));
    settings.result_fn = http_result_cb;

    // Configura e dispara a conexão via DNS
    cyw43_arch_lwip_begin();
    err_t err = httpc_get_file_dns(
        "api.thingspeak.com",
        80,
        uri,
        &settings,
        http_recv_cb,
        NULL,
        NULL
    );
    cyw43_arch_lwip_end();

    if (err == ERR_OK) {
        envio_pendente = true;
        printf("ThingSpeak: enviando alertas=%d...\n", alertas);
    } else {
        printf("ThingSpeak: erro ao iniciar (%d)\n", (int)err);
    }
}

// Lê o sensor interno da Pico e converte para graus Celsius
float ler_temperatura() {
    float soma = 0;
    for (int i = 0; i < 10; i++) { // Realiza média de 10 amostras para estabilidade
        adc_select_input(4);
        uint16_t raw = adc_read();
        const float conversion_factor = 3.3f / (1 << 12);
        float voltage = raw * conversion_factor;
        soma += 27 - (voltage - 0.706) / 0.001721; // Fórmula do datasheet
        sleep_us(50);
    }
    return soma / 10;
}

// Aciona o alerta visual (LED Vermelho) e sonoro (Buzzer com variação de frequência)
void disparar_alarme(ssd1306_t *p_display) {
    contador_intrusos++; // Incrementa log de segurança

    gpio_put(LED_RED, 1);
    gpio_put(LED_GREEN, 0);
    ssd1306_clear(p_display);
    ssd1306_draw_rect(p_display, 0, 0, 127, 63);
    ssd1306_draw_string(p_display, 35, 25, 1, "INTRUSO");
    ssd1306_show(p_display);

    for (int repeticao = 0; repeticao < 3; repeticao++) {
        for (int freq = 100; freq < 1000; freq += 50) {
            for (int j = 0; j < 10; j++) {
                gpio_put(BUZZER_A, 1); sleep_us(freq);
                gpio_put(BUZZER_A, 0); sleep_us(freq);
            }
        }
        for (int freq = 1000; freq > 100; freq -= 50) {
            for (int j = 0; j < 10; j++) {
                gpio_put(BUZZER_A, 1); sleep_us(freq);
                gpio_put(BUZZER_A, 0); sleep_us(freq);
            }
        }
        cyw43_arch_poll();
    }
}

// Lógica do jogo interativo utilizando o Joystick como controle
void jogar(ssd1306_t *p_display) { 

    // inicialização de variáveis e contagem regressiva 
    adc_select_input(1);
    srand(adc_read());
    int player_y   = 28;
    int wall_x     = 127;
    int gap_y      = 22;
    int gap_size   = 20;
    int score      = 0;
    int velocidade = 4;
    for (int c = 3; c > 0; c--) {
        char msg[4];
        sprintf(msg, "%d", c);
        ssd1306_clear(p_display);
        ssd1306_draw_string(p_display, 55, 25, 1, msg);
        ssd1306_show(p_display);
        sleep_ms(700);
    }
    while (true) {
        if (gpio_get(BOTAO_B) == 0) {
            sleep_ms(250);
            break;
        }
        adc_select_input(0);
        uint16_t y_val = adc_read();
        // Movimentação do jogador baseada no eixo Y do Joystick
        if (y_val > 3000 && player_y > 2)  player_y -= 3;
        if (y_val < 1000 && player_y < 57) player_y += 3;
        wall_x -= velocidade;
        if (wall_x < 0) {
            wall_x = 127;
            gap_y = rand() % 40;
            score++;
            if (score % 3 == 0 && velocidade < 9) velocidade++;
        }
        bool na_parede_x = (wall_x <= 7 && wall_x + 1 >= 4);
        bool no_buraco   = (player_y + 3 >= gap_y && player_y < gap_y + gap_size);

        // Verificação de colisão com os obstáculos (paredes)
        if (na_parede_x && !no_buraco) {

            // Game Over e reinicialização
            gpio_put(LED_RED, 1);
            for (int f = 200; f < 1000; f += 40) {
                gpio_put(BUZZER_A, 1); sleep_us(f);
                gpio_put(BUZZER_A, 0); sleep_us(f);
            }
            gpio_put(LED_RED, 0);
            char msg_score[20];
            sprintf(msg_score, "PONTOS: %d", score);
            ssd1306_clear(p_display);
            ssd1306_draw_rect(p_display, 0, 0, 127, 63);
            ssd1306_draw_string(p_display, 25, 15, 1, "GAME  OVER!");
            ssd1306_draw_string(p_display, 15, 35, 1, msg_score);
            ssd1306_show(p_display);
            sleep_ms(3000);
            player_y   = 28;
            wall_x     = 127;
            gap_y      = 22;
            score      = 0;
            velocidade = 2;
            continue;
        }
        ssd1306_clear(p_display);
        for (int x = 0; x < 128; x++) {
            ssd1306_draw_pixel(p_display, x, 0);
            ssd1306_draw_pixel(p_display, x, 63);
        }
        for (int px = 4; px <= 7; px++)
            for (int py = player_y; py <= player_y + 3; py++)
                ssd1306_draw_pixel(p_display, px, py);
        for (int py = 1; py <= 62; py++) {
            if (py < gap_y || py >= gap_y + gap_size) {
                ssd1306_draw_pixel(p_display, wall_x, py);
                if (wall_x + 1 < 128)
                    ssd1306_draw_pixel(p_display, wall_x + 1, py);
            }
        }
        char sc[8];
        sprintf(sc, "%d", score);
        ssd1306_draw_string(p_display, 100, 2, 1, sc);
        ssd1306_show(p_display);
        sleep_ms(50);
    }
}

bool conectar_wifi(ssd1306_t *p_display) {
    ssd1306_clear(p_display);
    ssd1306_draw_string(p_display, 10, 20, 1, "CONECTANDO WIFI");
    ssd1306_show(p_display);

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 15000)) {
        ssd1306_clear(p_display);
        ssd1306_draw_string(p_display, 20, 30, 1, "WIFI: ERRO!");
        ssd1306_show(p_display);
        sleep_ms(2000);
        return false;
    }

    ssd1306_clear(p_display);
    ssd1306_draw_string(p_display, 20, 30, 1, "WIFI: OK!");
    ssd1306_show(p_display);
    sleep_ms(1000);
    return true;
}

int main() {
    // Inicialização de periféricos (ADC, I2C, Wi-Fi e GPIOs)
    stdio_init_all();

    if (cyw43_arch_init()) {
        return -1;
    }

    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);
    adc_set_temp_sensor_enabled(true);

    gpio_init(BOTAO_A); gpio_set_dir(BOTAO_A, GPIO_IN); gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B); gpio_set_dir(BOTAO_B, GPIO_IN); gpio_pull_up(BOTAO_B);

    uint pins[] = {LED_RED, LED_BLUE, LED_GREEN, BUZZER_A, BUZZER_B};
    for (int i = 0; i < 5; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
    }

    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14); gpio_pull_up(15);

    ssd1306_t display;
    ssd1306_init(&display, 128, 64, i2c1, 0x3C);

    gpio_put(LED_BLUE, 1);
    for (int i = 0; i <= 100; i += 10) {
        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 25, 10, 1, "EMBARCATECH");
        ssd1306_draw_string(&display, 25, 22, 1, "SECURITY IOT");
        ssd1306_draw_rect(&display, 14, 45, 100, 10);
        for (int j = 0; j < i; j++) ssd1306_draw_pixel(&display, 14 + j, 47);
        ssd1306_show(&display);
        sleep_ms(150);
    }
    gpio_put(LED_BLUE, 0);

    while (true) {
        gpio_put(LED_RED, 0); gpio_put(LED_GREEN, 0); gpio_put(LED_BLUE, 0);
        gpio_put(BUZZER_A, 0);

        int selecao = 0;
        bool sistema_iniciado = false;

        // Menu de seleção: 0 para Segurança, 1 para Jogos
        while (!sistema_iniciado) {
            adc_select_input(0);
            uint16_t y_val = adc_read();
            if (y_val > 3000) selecao = 0;
            if (y_val < 1000) selecao = 1;

            ssd1306_clear(&display);
            ssd1306_draw_string(&display, 20, 5, 1, " SELECIONE: ");
            if (selecao == 0) { // MODO MONITORAMENTO
                ssd1306_draw_string(&display, 10, 30, 1, "> MONITORAMENTO");
                ssd1306_draw_string(&display, 20, 45, 1, "  JOGUINHO");
            } else {
                ssd1306_draw_string(&display, 20, 30, 1, "  MONITORAMENTO");
                ssd1306_draw_string(&display, 10, 45, 1, "> JOGUINHO");
            }
            ssd1306_show(&display);
            if (gpio_get(BOTAO_A) == 0) {
                sistema_iniciado = true;
                sleep_ms(250);
            }
            sleep_ms(100);
        }

        if (selecao == 0) {
            if (conectar_wifi(&display)) {
                while (true) {
                    
                    adc_select_input(1); uint16_t x_val = adc_read();
                    adc_select_input(0); uint16_t y_val = adc_read();

                    if (gpio_get(BOTAO_B) == 0) {
                        sleep_ms(250);
                        break;
                    }

                    // Monitora eixos do Joystick para detectar movimento
                    if (x_val < 1500 || x_val > 2500 || y_val < 1500 || y_val > 2500) {
                        disparar_alarme(&display);
                    } else {
                        gpio_put(LED_RED, 0);
                        gpio_put(LED_GREEN, 1);

                        float t = ler_temperatura();
                        int int_p = (int)t;
                        int dec_p = (int)((t - int_p) * 10);
                        if (dec_p < 0) dec_p *= -1;

                        char msg_temp[20];
                        char msg_alertas[20];
                        sprintf(msg_temp, "TEMP: %d.%d C", int_p, dec_p);
                        sprintf(msg_alertas, "ALERTAS: %d", contador_intrusos);

                        ssd1306_clear(&display);
                        ssd1306_draw_string(&display, 35, 5, 1, "SEGURANCA");
                        ssd1306_draw_string(&display, 20, 20, 1, "STATUS:ARMADO");
                        ssd1306_draw_string(&display, 20, 35, 1, msg_temp);
                        ssd1306_draw_string(&display, 20, 50, 1, msg_alertas);
                        ssd1306_show(&display);

                        
                        uint32_t agora = to_ms_since_boot(get_absolute_time());

                        // Status normal: LED Verde e telemetria para ThingSpeak
                        if (agora - ultimo_envio_ms >= INTERVALO_ENVIO_MS) {
                            ultimo_envio_ms = agora;
                            enviar_alertas_thingspeak(contador_intrusos);
                        }

                        cyw43_arch_poll(); // processa rede em background
                    }
                    sleep_ms(150);
                }
            }
        } else {
            jogar(&display);
        }
    }
}
