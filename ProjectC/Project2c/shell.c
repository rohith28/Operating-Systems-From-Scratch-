/*
Project Name : Project C _ Loading Files and Executing Programs
Name : Uppala Rohith Kumar
Version : 3.2

*/
void prnt(char buff);
void parseInput(char buffer);
void clear(char*,int);


main()
{
	char buffer[180];
	prnt("Shell is up.Please enter your command");
	prnt("\r\n");
	prnt("--> ");

	while(1)
       {
		
		interrupt(0x21,1,buffer,0,0);	
		prnt("\r\n");
		prnt("> ");
		parseInput(buffer);
		clear(buffer,180);

	}
}

void parseInput(char* buff)
       {
	char fileName[7];
	char fileBuff[13312];
	int bools = 1;
	int bufferCt = 0;
	int indexIn, index;
	int end;
	int i = 0;
	while(bools)
                {
		if (buff[bufferCt] == 0x0 || buff[bufferCt] == "\0"){
			
			bools = 0;
		}
		else
                {
			bufferCt++;
		}
	}
	for (indexIn = 0; indexIn <180; indexIn++)
             {

		if(buff[indexIn] != 0x0 && buff[indexIn] != '\r' && buff[indexIn] != '\n' && buff[indexIn] != '-' && buff[indexIn] != '>')
                {
			break;
		}
	}
	for (end=indexIn;end<180;end++)
            {

		if(buff[end] == '\r')
                {
			break;
		}
	}
	
	if (end - indexIn < 1)
        {
		prnt("Too short!");
	}
	else if (buff[indexIn]=='t' && buff[indexIn+1]=='y' && buff[indexIn+2]=='p' && buff[indexIn+3]=='e')
                {
		indexIn = indexIn + 5;

		for(i=0;i<6;i++)
                {
			fileName[i] = buff[indexIn+i];
		}
		fileName[6] = "\0";

		interrupt(0x21, 3, fileName, fileBuff, 0);

		prnt(fileBuff);
		prnt("--> ");
	}
	else if (buff[indexIn]=='e' && buff[indexIn+1]=='x' && buff[indexIn+2]=='e' &&  buff[indexIn+3]=='c' && 	buff[indexIn+4]=='u' && buff[indexIn+5]=='t' && buff[indexIn+6]=='e')
                {
		
		indexIn = indexIn + 8;

		for(i=0;i<6;i++)
                {
			fileName[i] = buff[indexIn+i];
		}
		fileName[6] = "\0";
		prnt(fileName);

		interrupt(0x21, 4, fileName, 0x2000, 0);
		
	}
	else
        {
		prnt("Recheck the Command - The Command not found!");
		prnt("\r\n");
		prnt("--> ");
	}
	
	
}

void prnt(char* buff)
{
	interrupt(0x21,0,buff,0,0);
}

void clear(char* buff, int len)
        {
	int i;
	for(i=0;i<len;i++)
        {
		buff[i] = 0x0;
	}
}

