#include <LiquidCrystal.h>;
#include <EEPROM.h>
LiquidCrystal lcd(8, 7, 5, 4, 3, 2); //rs, e, d4, d5, d6, d7

// ������ ���������

float umax          = 20.00;                     //������������ ����������
float umin          = 0.00;                      //����������� ����������
float ah            = 0.0000;                    //C������ �����*�����
const int down      = 10;                        //����� ��������� 1/2 �
const int up        = 11;                        //����� ��������� 2/2 �
const int pwm       = 9;                         //����� ��� 
const int pwm1      = 6;                         //����� ��� 
const int power     = 13;                        //���������� ������� 
long previousMillis = 0;                         //������ ����� ���������� ���������� �������
long maxpwm         = 0;                         //����� ��������� ������������� ���
long interval       = 500;                       // �������� ���������� ���������� �� �������, ��
int mig             = 0;                         //��� �������� (0 ����� 1 ���� 2 �����)
float level         = 2000;                      //"�������" ��� �������
byte  level2        = 0;                         //"�������" ��� �������
float com           = 100;
long com2           = 0;
int mode            = 0;                        //����� (0 �������, ������������ ����, ������ �� ����)
float Ioutmax       = 1.0;                      //�������� ���
int set             = 0;                        //������ ����, ����������� ������...
int knopka_a        = 0;                        //��������� ������
int knopka_b        = 0;
int knopka_c        = 0;
int knopka_abc      = 0;
boolean off         = false;
boolean red         = false;
boolean blue        = false;
float counter       = 5;                       // ���������� ������ �������� ����������
int disp            = 0;                       //����� ����������� 0 ������, 1 ���������, 2 �����, 3 ������������� ���, 4 ��� �������
float Uout ;                                   //���������� �� ������

#define kn_menu       12                       // ���������� ������ "����"
#define kn_selection  A5                       // ���������� ������ "�����"
#define kn_pwm        A4                       // ���������� ������ "���"

int incomingByte;


void EEPROM_float_write(int addr, float val) // ������ � ������
{  
  byte *x = (byte *)&val;
  for(byte i = 0; i < 4; i++) EEPROM.write(i+addr, x[i]);
}

float EEPROM_float_read(int addr) // ������ �� ������
{    
  byte x[4];
  for(byte i = 0; i < 4; i++) x[i] = EEPROM.read(i+addr);
  float *y = (float *)&x;
  return y[0];
}

//������� ��� �������� ��������
void uup()   //������� +
{ 
  if(set==0)
  {//������� ����� - ��������� ����������
	  if(mode == 0)
	  {
		 if(counter<umax) 
		 { 
		  counter = counter+0.1;//���������
		 }
	  }

	if(mode == 2)
	{
		level2++;
		if(level2 > 255) level2 = 255;
	}
  }
  if(set==1)
  { //����������� ����� ������ ������
   mode++;
   if(mode>2) mode=2; 
  }
  if(set==2)
  { //��������� ����, ��������� ���
   iplus();
  }
  
    if(set==3)
	{//����� �������� �*�
		ah = 0;
		set = 0;
		disp = 5;
	}

  if(set==4){//���������� ������� �������� � ������
save();
  }
}

void udn() //�������� -
{ //�������� -
  if(set==0)
  {
	if(mode == 0)
	{
		if(counter>umin+0.1)counter = counter-0.1; //�������� �����������
	}
	if(mode == 2)
	{
		level2--;
		if(level2 < 0) level2 = 0;
	}

  }

  if(set==1){
   mode--; //����������� ����� ������ �����
   if(mode<0) mode=0;
  }  
  if(set==2){//�������� ���
   iminus();
  }
}

void iplus()
{ 
    Ioutmax = Ioutmax+0.01;
   if(Ioutmax>0.2) Ioutmax=Ioutmax+0.04;
   if(Ioutmax>1) Ioutmax=Ioutmax+0.05;
   
   
   if(Ioutmax>10.00) Ioutmax=10.00;
}

void iminus(){ 
  Ioutmax = Ioutmax-0.01;
   if(Ioutmax>0.2) Ioutmax=Ioutmax-0.04;
   if(Ioutmax>1) Ioutmax=Ioutmax-0.05;
   if(Ioutmax<0.03) Ioutmax=0.03;
}

void save()
{
    lcd.clear();
    lcd.setCursor (0, 0);
    lcd.print(" S A V E  -  OK ");

     EEPROM_float_write(0, counter);
     EEPROM_float_write(4, Ioutmax);
     EEPROM_float_write(12, mode);
     EEPROM_float_write(10, disp);
	 EEPROM.write(16, level2);

     //������ ������������
    digitalWrite(A2, HIGH);  
    digitalWrite(A3,HIGH);  
    delay(500);
    digitalWrite(A2,LOW); 
    digitalWrite(A3,LOW);   
    set = 0; //������� �� ����
}

void setup()
{          
	cli();
	DDRB |= 1<<1 | 1<<2;
	PORTB &= ~(1<<1 | 1<<2);
	TCCR1A = 0b00000010;
	//TCCR1A = 0b10100010;
	TCCR1B = 0b00011001;
	ICR1H = 255;
	ICR1L = 255;
	sei();
	int pwm_rez = 13;
	pwm_rez = pow(2, pwm_rez);
	ICR1H = highByte(pwm_rez);
	ICR1L = lowByte(pwm_rez);

	Serial.begin(9600);  

	pinMode(pwm, OUTPUT);  // �� ����� 9
	pinMode(pwm, OUTPUT);  // �� ����� 6
	pinMode(down, INPUT);  
	pinMode(up, INPUT);  
	pinMode(kn_menu, INPUT); 
	pinMode(kn_selection, INPUT); 
	pinMode(kn_pwm, INPUT);
	pinMode(power, OUTPUT); 
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	// ������������ ������� �� ������ �� ���������
	digitalWrite(up, HIGH); 
	digitalWrite(down, HIGH);
	//������������ ������� �� ��������� ������
	digitalWrite(kn_menu, HIGH); 
	digitalWrite(kn_selection, HIGH);
	digitalWrite(kn_pwm, HIGH); 
	//������ �������
	lcd.begin(16, 2);     
	lcd.print("    WELCOME!    ");

	//��������� ��������� �� ������ ��
	counter = EEPROM_float_read(0);
	Ioutmax = EEPROM_float_read(4);
	mode = EEPROM_float_read(12);
	disp = EEPROM_float_read(10);
	level2 = EEPROM.read(16);

	//���� � ������ ��� ��� �������� - ������ ��� ������ ����� �����
	if(counter==0) counter = 5; //5 �����
	if(Ioutmax==0) Ioutmax = 2; //2 ������
  
	//�������� ����
	digitalWrite(power, HIGH);
}

void loop() //�������� ���� ������ ��
{
  
  unsigned long currentMillis = millis(); 
  
  /* ������ ���������� */
  if (Serial.available() > 0) {  //���� ���� ��������� ������
        // ��������� ����
        incomingByte = Serial.read();
 
    }else{
      incomingByte = 0;
    }
    
    if(incomingByte==97){ //a
    if(counter>umin+0.1)counter = counter-0.1; //�������� �����������
    
    }   
        if(incomingByte==98){ //b
  
    if(counter<umax)       counter = counter+0.1;//���������
     
    }
    
     if(incomingByte==99){ //c   
iminus();
     }
     
      if(incomingByte==100){ //d
         iplus();
      }

  if(incomingByte==101) mode = 0;
  if(incomingByte==102) mode = 1; 
  if(incomingByte==103) mode = 2;
  if(incomingByte==104) save();
  if(incomingByte==105){
    digitalWrite(power,HIGH); //������� ���� ���� ��� ���� ���������
    delay(100);
  digitalWrite(A2,LOW); //����� ������� ��������� 
    Serial.print('t');
    Serial.print(0);
    Serial.print(';');
  off = false;
  set = 0;//������� �� ����
  }
  
   if(incomingByte==106) off = true;
   if(incomingByte==107) ah = 0;
   
   /* ����� �������� ���������� */
   
   
  
  //�������� �������� ���������� � ���� � ��������
  float Ucorr = -0.00; //��������� ����������, ��� ������� ����� ����������
  float Uout = analogRead(A1) * ((5.0 + Ucorr) / 1023.0) * 5.0; //������ ���������� �� ������
    
  float Iout = analogRead(A0) / 100.00; // ������ ��� � ��������
  
  if(Iout==0.01) Iout =  0.03; else 
  if(Iout==0.02) Iout =  0.04; else
  if(Iout==0.03) Iout =  0.05; else
  if(Iout==0.04) Iout = 0.06; else
  if(Iout>=0.05) Iout = Iout + 0.02;
  if(Iout>=0.25)Iout = Iout + 0.01;
  //if(Iout>=1)Iout = Iout * 1.02;
  
  
  
  
  /* ������ � ���������� */
  
  if (((Iout>(counter+0.3)*2.0) | Iout>10.1  | off) & set<4 & millis()>100 ) // ������� ������

   {  
     digitalWrite(power, 0); //�������� ����
     level = 8190;           //������� ��� ������
     digitalWrite(A2, HIGH);      
    Serial.print('I0;U0;r1;W0;');
    Serial.println(' ');
    set = 6;
     
    }
    
    
    //������ �� ����������� ������������� ���
    if (level==0 & off==false)
    {  
      if(set<4)//���� ��� �� ��������� ������
      { 
        maxpwm--; //��������� +1 � ��������
        digitalWrite(A2, HIGH); //������ ������� ��� �������������� � ������������ ���
      }  
    }
    else //��� � ��� �� ������������, ������� �������� �������
    {
      maxpwm++;
      if(maxpwm>8190)//���� ������� ����� �� ����
      {
        maxpwm = 8190; //����� ��� � ������
        if(set<4) digitalWrite(A2, LOW); // ����� ������� ���������. ���������� ���.
      }
    }
  
  
  /* ������ ����� */
  
  
  // ��������� �������� � ����� ���������
  boolean regup   = digitalRead(up);
  boolean regdown = digitalRead(down);

  if(regup<regdown) mig = 1; // �������� � ������� ����������
  if(regup>regdown) mig = 2; // �������� � ������� ���������
  if(!regup & !regdown) //������ ��� ������������
  { 
    if(mig==1) uup();//+
    if(mig==2) udn(); //-
    mig = 0; //���������� ��������� �����������
  }

if(mode==0 | mode==1) //���� ��������� ������ ����������� (�� ����� ������������ ����)
{ 
  
  //���������� ���������� �� ������ � �������������, � ��������� ����..
  if(Uout>counter)
  {
    float raz = Uout - counter; //�� ������� ���������� �� ������ ������ ��������������...
    if(raz>0.05)
    {
      level = level - raz * 20; //������� ������� ��������� ����� � ������!
    }else{
       if(raz>0.015)  level = level -  raz * 3 ; //������� ��������� ��������� �����
    }
  }
  if(Uout<counter)
  {
    float raz = counter - Uout; //�� ������� ���������� ������ ��� �� �����
    if(raz>0.05)
    {
      level = level + raz * 20; //�����
    }else{
      if(raz>0.015)  level = level + raz * 3 ; //�����
    }
  }

  if(mode==1&&Iout>Ioutmax) //����� ������ �� ����, � �� ������ ��� �� ����������
  { 
   digitalWrite(power, LOW); //�������� ����
    Serial.print('t');
    Serial.print(2);
    Serial.print(';');
   
   //�������� ������� ���������
   digitalWrite(A2, 1);   
   level = 8190; //������� ��� ������
   set=5; //����� ����� � ������...
  }
  
}else{ //����� ������������ ����

  if(Iout>=Ioutmax)
  {
    //������ ����� ������� ����� ����� � �������� � ������������� �����
    float raz = (Iout - Ioutmax); 
    if(raz>0.3) //����� ������ ��������� (��� ������ ��������� ����� ��� �� 0,3�)
    {
      level = level + raz * 20; //����� �������� ���
    }else{    
      if(raz>0.05) //������ ��������� (��� ������ ��������� ����� ��� �� 0,1�)
      {
        level = level + raz * 5; //�������� ���
      }else{
        if(raz>0.00) level = level + raz * 2; //������� �������� (0.1 - 0.01�) �������� ������
      }
    }
  
   
  //�������� ����� ���������
  digitalWrite(A3,HIGH); 
}else{ //����� ������������ ����, �� ��� � ��� � �������� �����, � ������ ���������� ������������ ����������
digitalWrite(A3,LOW);//����� ��������� �� ��������

  //���������� ���������� �� ������ � �������������, � ��������� ����..
  if(Uout>counter)
  {
    float raz = Uout - counter; //�� ������� ���������� �� ������ ������ ��������������...
    if(raz>0.1)
    {
      level = level + raz * 20; //������� ������� ��������� ����� � ������!
    }else{
       if(raz>0.015)  level = level + raz * 5; //������� ��������� ��������� �����
    }
  }
  if(Uout<counter)
  {
    float raz = counter - Uout; //�� ������� ���������� ������ ��� �� �����
    float iraz = (Ioutmax - Iout); // 
    if(raz>0.1 & iraz>0.1)
    {
      level = level - raz * 20; //�����
    }else{
      if(raz>0.015)  level = level - raz ; //�����
    }
  }
 }
}//����� ������ ������������ ����

if(off) level = 8190;
if(level<0) level = 0; //�� �������� ��� ���� ����
if(level>8190) level = 8190; //�� ��������� ��� ���� 13 ���
//��� ���������, ��������� � ���������� ������ ������� ��� �������� �����������.
if(ceil(level)!=255) analogWrite(pwm, ceil(level)); //������ ������ ������ �� ��� ����� (����� 255, ��� ��� ��� �����-�� ����)

analogWrite(pwm1, level2); //������ ������ ������ �� ��� ����� (����� 255, ��� ��� ��� �����-�� ����)


/* ���������� */

if (digitalRead(kn_menu)==0 && digitalRead(kn_selection)==0 && digitalRead(kn_pwm)==0 && knopka_abc==0 ) { // ������ �� ������ a - � - c  ������
  knopka_abc = 1;

  //ah = 0.000;

  knopka_abc = 0;
}

if (digitalRead(kn_pwm)==0 && knopka_c==0) { // ������ �� ������ C (disp)
knopka_c = 1;
disp++; //���������� ����������� ����� ��� 1 � ��� 2
if(disp==6) disp = 0; //����� �� �����, �������� �����
}


if (digitalRead(kn_selection)==0 && knopka_a==0) { // ������ �� ������ � (disp)
knopka_a = 1;
disp++; //���������� ����������� ����� ����������� ����������
if(disp==6) disp = 0; //����� �� �����, �������� �����
}

if (digitalRead(kn_menu)==0 && knopka_b==0) { // ������ �� ������ � (menu)
knopka_b = 1;
set++; //
if(set>4 | off) {//������������ ���� �� ������� ������, � ���� ������� �� ��� ��������. (��� �� ������ ����� �� ����� ����)
off = false;
  digitalWrite(power, 1); //������� ���� ���� ��� ���� ���������
  delay(100);
  digitalWrite(A2, 0); //����� ������� ��������� 
    Serial.print('t');
    Serial.print(0);
    Serial.print(';');
    Serial.print('r');
    Serial.print(0);
    Serial.print(';');
    Serial.println(' ');
  set = 0;//������� �� ����
  }
  lcd.clear();//������ �������
}

//���������� �������� ������ ��� ����-�� ����� ����.
if(digitalRead(12)==1&&knopka_b==1) knopka_b = 0;
if(digitalRead(13)==1&&knopka_a==1) knopka_a = 0;
if(digitalRead(A4)==1&&knopka_c==1) knopka_c = 0;

/* COM PORT */

if(currentMillis - com2 > com) {
    // ��������� ����� ���������� ����������
    com2 = currentMillis;  
    
    //������� �����*����
    ah = ah + (Iout / 36000);

    Serial.print('U');
    Serial.print(Uout);
    Serial.print(';');
    
    Serial.print('I');
    Serial.print(Iout);
    Serial.print(';');
    
    Serial.print('i');
    Serial.print(Ioutmax);
    Serial.print(';');
    
    Serial.print('u');
    Serial.print(counter);
    Serial.print(';');
    
    Serial.print('W');
    Serial.print(level);
    Serial.print(';');
    
    Serial.print('c');
    Serial.print(ah);
    Serial.print(';');
    
    Serial.print('m');
    Serial.print(mode);
    Serial.print(';');
    
    Serial.print('r');
    Serial.print(digitalRead(A2));
    Serial.print(';');
    
    Serial.print('b');
    Serial.print(digitalRead(A3));
    Serial.print(';');
    

    
    Serial.println(' ');
  
  }  
  
  /* ��������� LCD */



if(set==0)
{
  //����������� �����  
    
  //������� ������������� ���������� �� �������
  lcd.setCursor (0, 1);
  lcd.print("U>"); 
  if(counter<10) lcd.print(" "); //��������� ������, ���� �����, ����� �� ������� ��������
  lcd.print (counter,1); //������� ������������� �������� ����������
  lcd.print ("V "); //����� ��� ��� ������ 
 
  //���������� ����������
   
  /*��������� �� ������ �� ������ ��������, ���� ������ ��
  ������� �������� �������� �� �������*/

  if(currentMillis - previousMillis > interval) {
    // ��������� ����� ���������� ����������
    previousMillis = currentMillis;  
    //������� ���������� �������� ���������� � ���� �� �������
  
    lcd.setCursor (0, 0);
    lcd.print("U=");
    if(Uout<9.99) lcd.print(" ");
    lcd.print(Uout,2);
    lcd.print("V I=");
    lcd.print(Iout, 2);
    lcd.print("A ");
  
    //�������������� ����������
    lcd.setCursor (8, 1);
    if(disp==0)
	{  //������  
      lcd.print("         ");   // ����������� ����������� ������ DHT11
    }    
    if(disp==1){  //��������
      lcd.print(" ");
      lcd.print (Uout * Iout,2); 
      lcd.print("W   ");
    }  
    if(disp==2)
	{  //����� ��
      if(mode==0)lcd.print   ("standart"); // ��� 1
      if(mode==1)lcd.print  ("shutdown");  // ������
      if(mode==2)lcd.print ("    drop");   // ��� 2
    }  
    if(disp==3){  //������������ ���
       lcd.print (" I>"); 
       lcd.print (Ioutmax, 2); 
       lcd.print ("A ");
    }
    if(disp==4){  // �������� ���
      lcd.print ("pwm:"); 
      lcd.print (ceil(level), 0); 
      lcd.print ("  ");
    }
    if(disp==5){  // �������� ���
      if(ah<1){
        //if(ah<0.001) lcd.print (" ");
        if(ah<=0.01) lcd.print (" ");
        if(ah<=0.1) lcd.print (" ");
        lcd.print (ah*1000, 1); 
      lcd.print ("mAh  ");
      }else{
        if(ah<=10) lcd.print (" ");
      lcd.print (ah, 3); 
      lcd.print ("Ah  ");
      }
    }
  }
}

	/* ��������� ���� */
	if(set==1)//����� ������
	{
	 lcd.setCursor (0, 0);
	 lcd.print("> MENU 1/4    ");
	 lcd.setCursor (0, 1);
	 lcd.print("mode: ");
	 //����� (0 �������, ������������ ����, ������ �� ����)
	 if(mode==0)  lcd.print("normal          ");
	 if(mode==1)  lcd.print("shutdown    ");
	 if(mode==2)  lcd.print("drop        ");
	}

	if(set==2)
	{//��������� ����
	 lcd.setCursor (0, 0);
	 lcd.print("> MENU 2/4   ");
	 lcd.setCursor (0, 1);
	 lcd.print("I out max: ");
	 lcd.print(Ioutmax);
	 lcd.print("A");
	}
	if(set==3)
	{//���������� ����� �� ���� ��������� ���������
	 lcd.setCursor (0, 0);
	 lcd.print("> MENU 3/4      ");
	 lcd.setCursor (0, 1);
	 lcd.print("Reset A*h? ->");
	}

	if(set==4)
	{
		//���������� ����� �� ���� ��������� ���������
	 lcd.setCursor (0, 0);
	 lcd.print("> MENU 4/4      ");
	 lcd.setCursor (0, 1);
	 lcd.print("Save options? ->");
	}
	/* ��������� ������ */
	if(set==5)
	{//������. ����� ����
		lcd.setCursor (0, 0);
		lcd.print("ShutDown!        ");
		lcd.setCursor (0, 1);
		lcd.print("Iout");
		lcd.print(">Imax(");
		lcd.print(Ioutmax);
		lcd.print("A)"); 
		level=0;
		Serial.print('I0;U0;r1;W0;');
		Serial.println(' ');
	}

 
 if(set==6)
 {
		 //������. ����� ���� ����������� ������� ����������
	Serial.print('I0;U0;r1;W0;');
	digitalWrite(A2, true);
	Serial.println(' ');
	level=0;
	lcd.setCursor (0, 0);
	if (off==false)
	{
		lcd.print("[   OVERLOAD   ]");
	lcd.setCursor (0, 1);
	//� ��������� ����� ��� ���������
    
		 if((Iout>(counter+0.3)*2.0) | Iout>10.0)
		 {
			Serial.print('t');
			Serial.print(1);
			Serial.print(';');
			lcd.print("  Iout >= Imax  ");
		 }
     
	 }
	else
	{
 	lcd.print("[      OFF     ]");
	lcd.setCursor (0, 1);
	Serial.print('t');
	Serial.print(4);
	Serial.print(';');
	}
 }
 
}