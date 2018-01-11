#include<stdio.h>

void printString(char * chars);
void readString(char * chars);
void readSector(char * buffer, int sector);
void writeSector(char* buffer,int sector);
void handleInterrupt21(int ax, int bx, int cx, int dx);
int mod(int a, int b);
int div(int a, int b);
void deleteFile(char* name);
void readFile(int* fileName,int* buffer);
void writeFile(char* name,char* buffer,int secNum);
int strComp(int* buffer, char* fileName);
void executeProgram(char* name, int segment);
void terminate();


int main(void)
{
	char buffer[13312];

	makeInterrupt21();

	interrupt(0x21, 4, "shell\0", 0x2000, 0);
	
	while(1);
	return 0;
}

void printString(char * chars)
{
	char al;
	char ah = 0xe;
	int ax;
	int i;

	for(i = 0; chars[i] != 0; i++)
        {
 		al = chars[i];
	 	ax = ah * 256 + al;
		interrupt(0x10, ax, 0, 0, 0);
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
void writeSector(char* buffer,int sector)
{
	int relativeSector=mod(sector,18)+1;
	int head=mod(sector/18,2);
	int track=sector/36;
	interrupt(0x13, 3 * 256 +1, buffer, track * 256 + relativeSector, head * 256+0);
}

void handleInterrupt21(int ax, int bx, int cx, int dx) 
{
	if(ax == 0)
        {
		printString(bx);
	}
        else if(ax == 1)
        {
		readString(bx);
	}
        else if(ax == 2)
        {
		readSector(bx, cx);
	}
        else if(ax == 3)
        {
		readFile(bx, cx);
	}
        else if(ax == 4)
        {
		executeProgram(bx, cx);
	}
        else if(ax == 5)
        {
		terminate();
	}
	else if(ax==6)
	{
		writeSector(bx,cx);
	}
	else if(ax==7)
	{
		deleteFile(bx);
	}
else if(ax==8)
	{
		writeFile(bx,cx,dx);
	}
        else
        {
		printString("This is a Error Message. !!!\0");
	}
}

int mod(int a, int b)
       {
	while(a >= b)
        {
		a = a-b;	
	} 
	return a;
}

int div(int a, int b)
        {
	int q = 0;
	while((q + 1) * b <= a)
        {
		q += 1;	
	}
	return q;
}

void readFile(char* fileName,char* buffer)
{
	int fileFound;
    	int nameCt = 0;
	int index, k,h;
	int sectors[27];
	int j = 0;
	int i;
	int buffAddress = 0;
	
    	readSector(buffer, 2);  

	fileFound = strComp(buffer,fileName);

	if (fileFound!=0)
                {
		index = fileFound*32+6;
		for (j=0;j<26;j++)
                {
			sectors[j] = buffer[index+j];
		}

		sectors[26] = 0;
		k = 0;
		while(sectors[k]!=0x0)
                {
			readSector(buffer+buffAddress,sectors[k]);
			buffAddress += 512;
			k++;
		}
		
	}
      else
               {
		printString("File Not Found!");
		return;
	       }		

}

void writeFile(char* name, char* buffer, int secNum){
	char map [512];
	char dir [512];	
	int sectors [26];
	int i = 0;
	int s = 0;
	interrupt(0x21, 2, map, 1, 0);
	interrupt(0x21, 2, dir, 2, 0);

	while(i<512){
		if(map[i] == 0x00){
			sectors[s] = i;
			s++;
			if(s==secNum){
				break;
			}
		}
		i++;
	}
	if(s<secNum){
		interrupt(0x21, 99, dir, 2, 0);//error
	}

	else{
		for (i = 0; i < s; ++i)
		{
			int c = sectors[i];
			map[c] = 0xFF;
		}
		interrupt(0x21, 6, map, 1, 0);//write map
		i = 0;
		for (; i < 512; i+=32)
		{
			if(dir[i] == 0x00){
				int j = i;
				int z = 0;//counter for 6 bytes of name
				while(z<6){
					if (name[z] != '\0'){
						dir[j] = name[j-i];
					}
					else{
						dir[j] = 0x00;
					}
					j++;
					z++;
				}
				for (j=6; j < 32; ++j)
				{
					if(j < secNum+6){
						dir[j+i] = sectors[j-6];
					}
					else{
						dir[j+i] = 0x00;
					}
				}
				break;
			}
		}
		
		for (i = 0; i < secNum; ++i)
		{
			interrupt(0x21, 6, buffer, sectors[i], 0);
			buffer += 512;
		}
		interrupt(0x21, 6, dir, 2, 0);//write directory

	}
}


int strComp(char* buffer, char* fileName)
              { 
	int i, j;
	int checkFound = 0;

	for (i = 0; i < 16; i++)
             {
		if (buffer[32*i] != 0x0)
                    {
			for (j=0; j < 6; j++)
                            {
				if (buffer[j+32*1] == 0x0 || buffer[j+32*1] == '\r' || buffer[j+32*1] == '\n' 					|| fileName[j] == 0x0 || fileName[j] == '\r' || fileName[j] == '\n')
                                {
					break;
				}
                                else if (buffer[j+32*i] == fileName[j])
                                {
					checkFound = 1;	
				}
                                else 
                                {
					checkFound = 0;
					break;
				}
				
			}
		 	
			if (checkFound == 1)
                        {
				 return i;
			}
			}
		}
		if (checkFound == 0)
                        {
			for (i=0;i<13312;i++)
                        {
				buffer[i] = 0x0;
			}
		 
			return 0;
		 }
}

void executeProgram(char* name, int segment)
{
	int i;
	char readingBuffer[13312];
	readFile(name, readingBuffer);
	for(i=0; i<13312; i++)
        {
		putInMemory(segment, i, readingBuffer[i]);
	}
	launchProgram(segment);
}

void deleteFile(char* filename)
{
    char mapbuf[512], dirbuf[512];
    // use static to initialize with all zero
    static char zerobuf[512];
    int i, j, n;
    // load map and directory into buffer
    readSector(mapbuf, 1);
    readSector(dirbuf, 2);
    for(i=0; i<512; i+=32)
    {
        // search filename in directory
        if(dirbuf[i] == filename[0] && dirbuf[i+1] == filename[1] && \
           dirbuf[i+2] == filename[2] && dirbuf[i+3] == filename[3] && \
           dirbuf[i+4] == filename[4] && dirbuf[i+5] == filename[5])
        {
            // delete file name in entry in directory
            for(n=i; n<(i+6); n++)
            {
                dirbuf[n] = 0x00;
            }
            // find occupied sector
            for(j=i+6; j<(i+31); j++)
            {
                // erase corresponding sector
                if(dirbuf[j] != 0x00)
                {
                    writeSector(zerobuf, dirbuf[j]);
                    // update map in sector #1
                    mapbuf[dirbuf[j]] = 0x00;
                }
            }
            // clear directory/file flag
            dirbuf[i+32] = 0x00;
        }
    }
    // write back to update disk
    writeSector(mapbuf, 1);
    writeSector(dirbuf, 2);
}


void terminate()
{
	char shell[6];
	shell[0] = 's';
	shell[1] = 'h';
	shell[2] = 'e';
	shell[3] = 'l';
	shell[4] = 'l';
	shell[5] = 0x0;
	executeProgram(shell, 0x2000);
}
