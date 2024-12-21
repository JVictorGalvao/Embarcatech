#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

// define portas do semaforo
#define SEMAFORO_VERMELHO 1
#define SEMAFORO_AMARELO 2
#define SEMAFORO_VERDE 3

// define portas para a acessibilidade
#define BOTAO 12
#define LUZ_PEDESTRE 14
#define BUZZER_PEDESTRE 10

// Configuração da frequência do buzzer (em Hz)
#define FREQUENCIA_BUZZER 100

// inicia o estado do botão como não pressionado
volatile bool estado_botao = false;

// comportamento do semáforo
void semaforo(int vermelho, int amarelo, int verde, int pedestre, int buzzer){
  gpio_put(verde, 1);
  sleep_ms(5000);
  gpio_put(verde, 0);
  //verifica o estado do botão antes da transição do vermelho para confirmar se o botão foi pressionado
  if(estado_botao == false){
    gpio_put(amarelo, 1);
    sleep_ms(2000);
    gpio_put(amarelo, 0);
    gpio_put(vermelho, 1);
    sleep_ms(10000);
    gpio_put(vermelho, 0);
  }else{
    gpio_put(amarelo, 1);
    sleep_ms(5000);
    gpio_put(amarelo, 0);
    gpio_put(vermelho, 1);
    gpio_put(pedestre, 1);
    beeps(buzzer, 1500, 15000);
    gpio_put(pedestre, 0);
    gpio_put(vermelho, 0);
    estado_botao = false;
  }
}

void pwm_init_buzzer(uint pin) {
  // Configurar o pino como saída de PWM
  gpio_set_function(pin, GPIO_FUNC_PWM);

  // Obter o slice do PWM associado ao pino
  uint slice_num = pwm_gpio_to_slice_num(pin);

  // Configurar o PWM com frequência desejada
  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (FREQUENCIA_BUZZER * 4096)); // Divisor de clock
  pwm_init(slice_num, &config, true);

  // Iniciar o PWM no nível baixo
  pwm_set_gpio_level(pin, 0);
}

// Definição de uma função para emitir beeps com duração especificada
void beeps(uint pin, uint ciclo_ms, uint duracao_ms) {
  // Obter o slice do PWM associado ao pino
  uint slice_num = pwm_gpio_to_slice_num(pin);

  // Varia o estado do buzzer com a duracao em s especificada
  for(int i = 0; i < duracao_ms; i = i + ciclo_ms){
    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(ciclo_ms/2);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(ciclo_ms/2);
  }

}

// interrupção para indicar se o botão foi pressionado
void button_callback(uint32_t events) {
  estado_botao = true;
}


int main() {
  // inicializacao
  stdio_init_all();
  gpio_init(SEMAFORO_VERMELHO);
  gpio_init(SEMAFORO_AMARELO);
  gpio_init(SEMAFORO_VERDE);
  gpio_init(LUZ_PEDESTRE);
  gpio_init(BOTAO);

  gpio_set_dir(SEMAFORO_VERMELHO, GPIO_OUT);
  gpio_set_dir(SEMAFORO_AMARELO, GPIO_OUT);
  gpio_set_dir(SEMAFORO_VERDE, GPIO_OUT);
  gpio_set_dir(LUZ_PEDESTRE, GPIO_OUT);
  gpio_set_dir(BOTAO, GPIO_IN);
  gpio_pull_up(BOTAO);

  // Inicializar o PWM no pino do buzzer
  pwm_init_buzzer(BUZZER_PEDESTRE);

  // Adicionando a interrupção ao pino
  gpio_set_irq_enabled_with_callback(BOTAO, GPIO_IRQ_EDGE_FALL, true, &button_callback);

  while (true) {
      semaforo(SEMAFORO_VERMELHO, SEMAFORO_AMARELO, SEMAFORO_VERDE, LUZ_PEDESTRE, BUZZER_PEDESTRE);
  }
}
