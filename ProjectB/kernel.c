/*
Project Name : Project B _ System Calls
Name : Uppala Rohith Kumar
Version : 2.2

*/

void printString(char*);
int readString(char* line);
int mod(int a,int b);
int div(int c,int d);
void handleInterrupt21(int ax,int bx,int cx,int dx);
void readSector(char* buffer,int sector); 

int main(){
	char line[80];
	char buffer[512];
	printString("Hello World");
	printString("                        ");
	makeInterrupt21();
	interrupt(0x21,0,"Enter a line:",0,0);
	interrupt(0x21,1,line,0,0);
	interrupt(0x21,0,line,0,0);
	interrupt(0x21,2,buffer,30,0);
	interrupt(0x21,0,buffer,0,0);

while(1){}
return 0;
}

void printString(char* string)
{
	int i=0;
	while(*(string+i)!='\0'){
		char al=*(string+i);
		 char ah=0xe;
		int ax=ah*256 + al;
		interrupt(0x10,ax,0,0,0);
	i++;

	}
}

void readString(char * chars)
      {
	char a;
	int i = 0;
	char ah = 0xe;
	while(i < 77)
              {
 		a = interrupt(0x16, 0, 0, 0, 0);

  /* if enter pressed*/
		if(a == 0xd)
                {
			break;
		}
 /* if backspace pressed*/
else if(a == 0x8)
               {
			if(i > 0)
                        {
/* To print the backspace, and then a whitespace and then again print the backspace to remove the character */	
				interrupt(0x10, ah * 256 + a, 0, 0, 0);
				interrupt(0x10, ah * 256 + ' ', 0, 0, 0);
				interrupt(0x10, ah * 256 + a, 0, 0, 0);
				i--;
			}
		}
/* if normal character */
else
                {
			chars[i] = a;
			interrupt(0x10, ah * 256 + a, 0, 0, 0);
			i++;
		}
	}

	interrupt(0x10, ah * 256 + 0xa, 0, 0, 0);
	interrupt(0x10, ah * 256 + '\r', 0, 0, 0);
	chars[i] = 0xa;
	chars[i+1] = '\r';
	chars[i+2] = 0x0;
}
int mod(int a,int b)
{
	while(a>=b)
	{
		a=a-b;
	}
	return a;
}
int div(int c,int d)
{
	int q=0;
	while(q*c<=d)
	{		 
		q=q+1;
	}
	return q-1;
}
void readSector(char * buffer, int sector)
{
	int ah = 2;
	int al = 1;
	int ch = div(sector, 36);
	int cl = mod(sector, 18) + 1;
	int dh = mod(div(sector, 18), 2);
	int dl = 0;
	int ax = ah * 256 + al;
	int cx = ch * 256 + cl;
	int dx = dh * 256 + dl;
	interrupt(0x13, ax, buffer, cx, dx);
}

void handleInterrupt21(int ax, int bx, int cx, int dx)
{
	if(ax==0)
	{
		printString(bx);
	}
	else if(ax==1)
	{
		readString(bx);
	}
	else if(ax==2)
	{
		readSector(bx,cx);
		printString("If this message prints out, then your readSector function is working correctly!");
	
	}
	else{
		printString("It is in Handle Interrupt");
	}
}

