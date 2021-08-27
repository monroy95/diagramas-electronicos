// Mario Monroy - monroy95

// Declaracion de puertos
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RB0_bit;
sbit LCD_D5 at RB1_bit;
sbit LCD_D6 at RB2_bit;
sbit LCD_D7 at RB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;

//pines de comunicacion
sbit SHT_Reloj at RC0_bit;
sbit SHT_Datos at RC1_bit;

//declaracion de TRIS de los pines de comunicacion
sbit SHT_Reloj_Dire at TRISC0_bit;
sbit SHT_Datos_Dire at TRISC1_bit;

//constantes
const unsigned short RH_8bits_TEM_12bits = 1;
const unsigned short RH_12bits_TEM_14bits = 0;
const unsigned short Peso_bits[8] = {1, 2, 4, 8, 16, 32, 64, 128};

//bandera resolucion
unsigned short RH_TEM;
//funcion para establecer un estado logico en el pin de datos
void SHT_Bus(unsigned short e)
{
    //si el estado es 1 se impone en alta, impedancia el pin de datos
    if (e)
        SHT_Datos_Dire = 1;
    else
    {
        //si el estado es un 0 logico, se establece la salida en 0
        SHT_Datos_Dire = 0;
        SHT_Datos = 0;
    }
}

//FUNCION PARA ESTABLECER EL ESTADO DEL RELOJ
void SHT_Reloj_Salida(unsigned short e)
{
    //se establece el estado logico en el reloj
    if (e)
        SHT_Reloj = 1;
    else
        SHT_Reloj = 0;
    //periodo del reloj, para frecuencia de 1Mhz
    delay_us(1);
}

//FUNCION PARA CREAR LA CONDICION DE INICIO
void SHT_Inicio(void)
{
    //secuencia de inicio
    SHT_Reloj_Salida(1);
    SHT_Bus(0);
    SHT_Reloj_Salida(0);
    SHT_Reloj_Salida(1);
    SHT_Bus(1);
    SHT_Reloj_Salida(0);
    delay_ms(1);
}

//FUNCION PARA ESCRIBIR UN DATO EN EL BUS
void SHT_Escribe_Bus(unsigned short d)
{
    unsigned short n;
    //bucle para el envio serial
    for (n = 7; n != 255; n--)
    {
        //se asigna el bit segun su peso
        if ((d >> n) & 1)
            SHT_Bus(1);
        else
            SHT_Bus(0);
        //generacion de pulsos de reloj
        SHT_Reloj_Salida(1);
        SHT_Reloj_Salida(0);
    }
    //Gneracon de bit ACK
    SHT_Bus(1);
    SHT_Reloj_Salida(1);
    SHT_Reloj_Salida(0);
}
//Funcion para leer un dato en el BUS
unsigned short SHT_Leer_Bus(void)
{
    unsigned short n, Dato_SHT = 0;
    //se configura el bus de entrada
    SHT_Bus(1);
    //bucle para recibir los datos de forma serial
    for (n = 7; n != 255; n--)
    {
        //pulso de reloj
        SHT_Reloj_Salida(1);
        //se reciben los bit segun su peso
        if (SHT_Datos)
            Dato_SHT += Peso_bits[n];
        SHT_Reloj_Salida(0);
    }

    //se transmite la condicion de ACK
    SHT_Bus(0);
    SHT_Reloj_Salida(1);
    SHT_Reloj_Salida(0);
    //se deja el bus de entrada
    SHT_Bus(1);
    //se retorna el valor de la lectura
    return Dato_SHT;
}

//FUNCION PARA GRABAR EL ESTADO ESTATUS
void SHT_Status_Graba(unsigned short s)
{
    //se transmite el valor de status
    SHT_Inicio();
    SHT_Escribe_Bus(0b00000110);
    SHT_Escribe_Bus(s);
}

//FUNCION PARA LEER EL VALOR DE STATUS
unsigned short SHT_Status_Leer(void)
{
    unsigned short st;
    SHT_Escribe_Bus(0b00000111);
    st = SHT_Leer_Bus();
    SHT_Leer_Bus();
    return st;
}

//FUNCION PARA LEER LA TEMPERATURA
float SHT_Leer_Tem(void)
{
    float tem;
    unsigned int LEC = 0;
    unsigned short DH, DL;
    //se aplica el protocolo de lectura
    SHT_Inicio();
    SHT_Escribe_Bus(0b00000011);
    //espera para hacer la lectura
    delay_ms(310);
    DH = SHT_Leer_Bus();
    DL = SHT_Leer_Bus();
    SHT_Leer_bus();
    LEC = DL + DH * 256;
    //se aplican las ecuacion para la temperatura
    if (RH_TEM)
        tem = -40.1 + 0.04 * LEC;
    else
        tem = -40.1 + 0.01 * LEC;
    //se retorna la temperatura
    return tem;
}

float SHT_Leer_Hum(void)
{
    float hum;
    unsigned int LEC = 0;
    unsigned short DH, DL;
    //se aplica el protocolo de lectura
    SHT_Inicio();
    SHT_Escribe_Bus(0b00000101);
    //espera para hacer la lectura
    delay_ms(310);
    DH = SHT_Leer_Bus();
    DL = SHT_Leer_Bus();
    SHT_Leer_Bus();
    LEC = DL + DH * 256;
    //se implementan las ecuaciones para la humedad
    if (RH_TEM)
        hum = -4.3468 + 0.5872 * LEC - 0.00040845 * LEC * LEC;
    else
        hum = -4.3468 + 0.0367 * LEC - 0.0000015955 * LEC * LEC;
    //se retorna la humedad
    return hum;
}

//FUNCION PARA INICIALIZAR EL SENSOR
void inicio_SHT7x(unsigned short St)
{
    unsigned short n;
    //configuracion de pines y estado inicial del bus de datos
    SHT_Reloj_Dire = 0;
    //se estable el pin de datos en 1
    SHT_Bus(1);
    //se inicia el reloj en 0
    SHT_Reloj_Salida(0);
    delay_ms(100);
    //se generan 9 ciclos de reloj, para reinciar el sensor
    for (n = 0; n < 9; n++)
    {
        SHT_Reloj_Salida(1);
        SHT_Reloj_Salida(0);
    }
    //se configura el estado status
    SHT_Inicio();
    SHT_Status_Graba(St);
    delay_ms(1);
    RH_TEM = St;
}
void main()
{
    float sen;
    char text[16];
    Lcd_Init();
    Inicio_SHT7x(RH_12bits_TEM_14bits);
    Lcd_Cmd(_Lcd_Cursor_Off);
    Lcd_Out(1, 1, "TEM:");
    lcd_Out(2, 1, "HUM:");

    while (1)
    {
        sen = SHT_Leer_Tem();
        FloatToStr(Sen, text);
        Lcd_Out(1, 6, text);

        sen = SHT_Leer_Hum();
        FloatToStr(sen, text);
        Lcd_Out(2, 6, text);
    }
}