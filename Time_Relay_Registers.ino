/* Hardware maping:
 *  
 *  Buttons:
 *    BT0 = PB8 
 *    BT1 = PB4
 *    BT2 = PB5
 *    BT3 = PB6
 *    
 *  LEDs:
 *    LED_OB = PC13 (Onboard)
 *    LED0   = PA7  //AZUL
 *    LED1   = PA8  //verde
 *    LED2   = PA9  //amerelo
 *    LED3   = PA10 //vermelho
 *    LED4   = PA15 //branco
 * 
 *  Decoder BCD to 7 segment:
 *    a   = PA1
 *    b   = PA2
 *    c   = PA3
 *    d   = PA4
 *    DPA = PA5
 * 
 *  7 segment cathode:
 *    1° Digit = PB0
 *    2° Digit = PB1
 *    3° Digit = PB10
 *    4° Digit = PB11
 */

void readKey();
void delay_micros(unsigned long us);
void disp(unsigned char number);                      //converte número em dígito para 7 segmentos catodo comum
void multiplex();
void countDown();

boolean startFlag   = 0;
int8_t  casaDec     = 0;
int16_t lastVal     = 0,
        val         = 0;
        

void timerInterrupt(){

  static int count = 0;                              //variável local para base de tempo
  count += 1;    
  multiplex();
  
  TIMER2_BASE->SR &= 0xFFFE;                         //limpa a flag de interrupção

  if(count == 500)                                   //count igual a 500?
  {                                                  //sim, passou 1 segundo (2ms x 500)
    if(startFlag) countDown();                       //chama a função de decremento, caso o ciclo esteja ativado
    count = 0;                                       //reinicia count
    GPIOC_BASE->ODR ^= 0x2000;                       //blink LED onboard
  } //end if
  
}

void setup() {
//======================================= Port A =============================================//
  
  GPIOA_BASE->CRH  = 0x24444222;                      //PA8/9/10/15   COMO SAIDAS 
  GPIOA_BASE->CRL  = 0x24222224;                      //PA1/2/3/4/5/7 COMO SAIDAS
  GPIOA_BASE->ODR  = 0x0000;                          //DESLIGA SAIDAS PA

//======================================= Port B =============================================//
  
  GPIOB_BASE->CRH = 0x44442248;                       //PB10/11   COMO SAIDAS   && PB8 ENTRADA
  GPIOB_BASE->CRL = 0x48884422;                       //PB4/5/6   COMO ENTRADAS && PB0/1 COMO SAIDA
  GPIOB_BASE->ODR = 0x0170;                           //PB4/5/6/8 COMO PULL_UP  && DESLIGA RESTO
  
//======================================= Port C =============================================//

  GPIOC_BASE->CRH  = 0x44244444;                      //PC13    SAIDA(LED ONBOARD)
  GPIOC_BASE->BSRR = 0x20000000;                      //DESLIGA SAIDA PC13;

//======================================= Timer 2 =============================================//

  TIMER2_BASE->PSC = 7200;                            //configura Prescaler do Timer2: (1/72E6) x 7200 = 100µs 
  TIMER2_BASE->ARR =   20;                            //prescaler ARR para base de tempo mais lenta: 100µs x 20 = 2ms
  TIMER2_BASE->CNT =    0;                            //inicializa Timer2 em 0
  timer_attach_interrupt(TIMER2, 0, timerInterrupt);  //habilita interrupção do Timer2 com a função "my_isr"
  TIMER2_BASE->CR1 |= 0x0001;                         //habilita Timer2  

//=============================================================================================//


  

}

void loop() {

  readKey();                                           //função de leitura de botão
  

}

void countDown()
{
  if(val>0)
  {
    val-=1;                                        //decrementa a variavel de ciclo
  }else if(val==0)                                 //desliga o ciclo e retorna o valor de ciclo para seu estado de inicio;
  {
    startFlag = 0;
    GPIOA_BASE->BSRR = 0x00008000;                 //liga a saida pre setada do microcontrolador
    val = lastVal;
  }
}

void readKey()
{
  static boolean  b1 = 0, b2 = 0, b3 = 0,b4 = 0;
  uint8_t y;

  if(casaDec==0)                                     //lógica que adequa a soma dos valores de acordo a casa decimal escolhida.
  {
    y = 0;
  }else if(casaDec==1)
  {
    y = 1;
  }else if(casaDec==2)
  {
    y = 10;
  }else if(casaDec==3)
  {
    y = 100;
  }
if(!startFlag)
{  
  //============================== b1 mais ==================================//

  if(!(GPIOB_BASE->IDR & (1<<4)) && !b1)               //bt1 pressionado?
  {                                                    //sim
    b1 = 1;
    delay_micros(1000);
  }
  
  if((GPIOB_BASE->IDR & (1<<4)) && b1)                 //bt1 solto?
  {
    GPIOA_BASE->ODR ^= 0x0100;                         //INVERTE LED1
    b1 = 0;
    if(casaDec==4)                                     //lógica de soma de valores para com o valor de ciclo
    {
      val += 1000;
    }else val += y;
    if(val>9999)
    {
      val = 0;
    }
    delay_micros(1000);
  }
  
  //============================== b2 menos ==================================//

  if(!(GPIOB_BASE->IDR & (1<<5)) && !b2)               //bt2 pressionado?
  {                                                    //sim
    b2 = 1;
    delay_micros(1000);
  } 
  
  if((GPIOB_BASE->IDR & (1<<5)) && b2)                 //bt2 solto?
  {
    GPIOA_BASE->ODR ^= 0x0200;                         //INVERTE LED2
    b2 = 0;
    if(casaDec==4)                                     //lógica de subtração de valores para com o valor de ciclo
    {
      val -= 1000;
    }else val -= y;
    if(val<0)
    {
      val = 9999;
    }
    delay_micros(1000);
  }
  
  //============================== b3 digito ==================================//

  if(!(GPIOB_BASE->IDR & (1<<6)) && !b3)               //bt3 pressionado?
  {                                                    //sim
    b3 = 1;
    delay_micros(1000);
  }
  
  if((GPIOB_BASE->IDR & (1<<6)) && b3)                 //bt3 solto?
  {
    GPIOA_BASE->ODR ^= 0x0400;                         //INVERTE LED3
    b3 = 0;
    
    if(casaDec+1<=4)casaDec += 1;                      //incremento da variável de casa decimal para modificação
    else casaDec = 0;
    
    delay_micros(1000);
  }
}  
  //============================== b4 mais ==================================//

  if(!(GPIOB_BASE->IDR & (1<<8)) && !b4)               //bt4 pressionado?
  {                                                    //sim
    b4 = 1;
    delay_micros(1000);
  }
  
  if((GPIOB_BASE->IDR & (1<<8)) && b4)                 //bt1 solto?
  {
    GPIOA_BASE->ODR ^= 0x0080;                         //INVERTE LED azul
    casaDec = 0;                                       //desliga a casa decimal, para não aprecer o ponto no display
    if(!startFlag)
    {
      GPIOA_BASE->BSRR = 0x80000000;                   //desliga LED branco de acionamento
      lastVal = val;                                   //salva o valor atual em uma variavel do sistema
      if(val!=0)startFlag = 1;                         //torna a variavel de inicio em verdadeira, dando inicio ao ciclo CASO VALOR DE CICLO SEJA DIFERENTE DE ZERO
    }else{ val = lastVal;startFlag = 0; }              //retorna o variavel de valor, para seu estado inicial do ciclo  e desliga o ciclo
    
    b4 = 0;
    delay_micros(1000);
  }
   
} //end countdown

void multiplex()//char dig, char num)
{
  static uint16_t umil,cen,dez,uni;
  uint8_t x = 0;
  String corteValor = String(val);
  static char dig = 0;
 
  if(val>=1000)                                     // lógica de reposicionamento da leitura da String do valor, a cada casa decimal decrementada
  {                                                 //é deslocado uma casa para a direita na string para que a leitura se mantenha correta.  
    x = 0;
  }else if(val<1000 && val>=100)
  {
    x = 1;
  }else if(val<100 && val>=10)
  {
    x = 2;
  }else if(val<10 && val>=0)
  {
    x = 4;                                            
  }

  dig += 1;
  if(dig>4) dig = 1;                                  //somador do digito, para rotacionar os acionamentos de cada digito do display

  
  
  switch(dig)                                         //Verifica qual o digito
  {
    case 1:                                
      GPIOB_BASE->BSRR  = 0x0C020001;                 // 0 liga umil, desliga cen dez uni
      if(casaDec==4)
      {
        GPIOA_BASE->BSRR  = 0x003E0020;  
      }else GPIOA_BASE->BSRR  = 0x003E0000;           //limpa barramento de segmentos
      umil = corteValor.substring(0-x,1-x).toInt();   //retira da string a unidade de mil
      disp(umil);                                     //imprime unidade de mil
      break;                                          //encerra laço
      
    case 2:                                           
      GPIOB_BASE->BSRR  = 0x0C010002;                 // 1 liga cen, desliga umil dez uni
      if(casaDec==3)
      {
        GPIOA_BASE->BSRR  = 0x003E0020;  
      }else GPIOA_BASE->BSRR  = 0x003E0000;           //limpa barramento de segmentos
      cen = corteValor.substring(1-x,2-x).toInt();    //retira da string unidade
      disp(cen);                                      //imprime unidade
      break;                                          //encerra laço
      
    case 3:                                           
      GPIOB_BASE->BSRR  = 0x08030400;                 //10 liga dez, desliga umil cen uni
      if(casaDec==2)
      {
        GPIOA_BASE->BSRR  = 0x003E0020;  
      }else GPIOA_BASE->BSRR  = 0x003E0000;           //limpa barramento de segmentos
      dez = corteValor.substring(2-x,3-x).toInt();    //retira da string unidade
      disp(dez);                                      //imprime unidade
      break;                                          //encerra laço
      
    case 4:                                          
      GPIOB_BASE->BSRR  = 0x04030800;                 //11 liga uni, desliga umil cen dez
      if(casaDec==1)
      {
        GPIOA_BASE->BSRR  = 0x003E0020;  
      }else GPIOA_BASE->BSRR  = 0x003E0000;           //limpa barramento de segmentos
      uni = corteValor.substring(3-x,4-x).toInt();    //retira da string unidade
      disp(uni);                                      //imprime unidade
      break;      
      
  } //end switch digito
  
} //end multiplex

void disp(int number)
{
  if(number==0){
    GPIOA_BASE->BSRR = 0x00000000;                  //seta bits para gerar '0' - PA1/2/3/4 -> / 0 0 0 0 / 0 0 0 0 /
  }else if(number==1){
    GPIOA_BASE->BSRR = 0x00000002;                  //seta bits para gerar '1' - PA1/2/3/4 -> / 0 0 0 0 / 0 0 1 0 /
  }else if(number==2){
    GPIOA_BASE->BSRR = 0x00000004;                  //seta bits para gerar '2' - PA1/2/3/4 -> / 0 0 0 0 / 0 1 0 0 /
  }else if(number==3){
    GPIOA_BASE->BSRR = 0x00000006;                  //seta bits para gerar '3' - PA1/2/3/4 -> / 0 0 0 0 / 0 1 1 0 /
  }else if(number==4){
    GPIOA_BASE->BSRR = 0x00000008;                  //seta bits para gerar '4' - PA1/2/3/4 -> / 0 0 0 0 / 1 0 0 0 /
  }else if(number==5){
    GPIOA_BASE->BSRR = 0x0000000A;                  //seta bits para gerar '5' - PA1/2/3/4 -> / 0 0 0 0 / 1 0 1 0 /
  }else if(number==6){
    GPIOA_BASE->BSRR = 0x0000000C;                  //seta bits para gerar '6' - PA1/2/3/4 -> / 0 0 0 0 / 1 1 0 0 /
  }else if(number==7){
    GPIOA_BASE->BSRR = 0x0000000E;                  //seta bits para gerar '7' - PA1/2/3/4 -> / 0 0 0 0 / 1 1 1 0 /
  }else if(number==8){
    GPIOA_BASE->BSRR = 0x00000010;                  //seta bits para gerar '8' - PA1/2/3/4 -> / 0 0 0 1 / 0 0 0 0 /
  }else if(number==9){
    GPIOA_BASE->BSRR = 0x00000012;                  //seta bits para gerar '9' - PA1/2/3/4 -> / 0 0 0 1 / 0 0 1 0 /
  }
} //end disp


void delay_micros(unsigned long us)
{
  static unsigned long a,b,c,d;                       //variáveis locais para operações matemáticas de atraso
  register unsigned long i;                           //variável para iterações
 
  for(i=0;i<(us*5);i++)                               //faz us*5 iterações
  {
     a=b*i;                                           //cálculo matemático aleatório para gastar tempo
     c=c*i;                                           //cálculo matemático aleatório para gastar tempo 
     d=d*i;                                           //cálculo matemático aleatório para gastar tempo 
     c=a*i;                                           //cálculo matemático aleatório para gastar tempo
     b=a*c*d*c;                                       //cálculo matemático aleatório para gastar tempo
    
  } //end for us
     
} //end delay_micros
