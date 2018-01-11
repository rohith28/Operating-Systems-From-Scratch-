void prnt(char buff);
void parseInput(char buffer);
void clear(char*,int);
void doView(char*);
void doExecute(char*);
void doDelete(char*);
void doDir();
void doCopy(char*);
void doCreate(char*);
int cmdNumber(char*);
int MOD(int, int);
int DIV(int, int);

char shellBuf[13312];
char readBuf[80];

main()
{
	int cmdNum;
	int iShell;
	char cmd[80];

	while(1){

		// Before each commant the shellBuffer is emptied for safety purposes
		for(iShell=0; iShell<13312; iShell++) shellBuf[iShell] = 0x0;
		interrupt(0x21, 0, ">SHELL$\0", 0, 0);
		interrupt(0x21, 1, cmd, 0, 0);
		interrupt(0x21, 0, "\n\r\0", 0, 0);
		cmdNum = cmdNumber(cmd);

		/* If the user pressed enter with nothing writted he'll get a 
		 * new "A:>" without a "Bad Command !!" message as he entered NO command. 
		 */
		if( *cmd==0xD ) continue;

		// Parsing command type
		if( cmdNum == 0 ) doView(cmd);
 		else if( cmdNum == 1 ) doExecute(cmd);
		else if( cmdNum == 2 ) doDelete(cmd);
		else if( cmdNum == 3 ) doDir();			
		else if( cmdNum == 4 ) doCopy(cmd);
		else if( cmdNum == 5 ) doCreate(cmd);
		else interrupt(0x21, 0, "Bad Command !!\0", 0, 0);
		
	}

	return 0;
	/*char buffer[180];
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

	}*/
}

/*void parseInput(char* buff)
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
*/
void prnt(char* buff)
{
	interrupt(0x21,0,buff,0,0);
}
void doDelete(char* CMD){

	int iShell;
	char fName1[7];

	// Parsing filename
	iShell = 0;
	while( iShell<7 && *(CMD+7+iShell)!=0xD && *(CMD+7+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+7+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }

	interrupt(0x21, 3, fName1, shellBuf, 0);
	if( *shellBuf==0x0 ) interrupt(0x21, 0, "DELETE-ERROR 0xA3: Either file not exists or you've not specified a name.\0", 0, 0);
	
	else {
		// Apply Delete on Interrupt 7.
		interrupt(0x21, 7, fName1, 0, 0);

		// If the file has not been deleted; It must have been either the shell or KERNEL.
		for(iShell=0; iShell<13312; iShell++) shellBuf[iShell] = 0x0;
		interrupt(0x21, 3, fName1, shellBuf, 0);
		if( *shellBuf!=0x0 ) return; 

		// Else the file has been deleted successfully.
		interrupt(0x21, 0, "File: \"\0", 0, 0);
		interrupt(0x21, 0, fName1, 0, 0);
		interrupt(0x21, 0, "\" deleted successfully. \0", 0, 0);
	}


}
void doDir(){

	int iShell;
	int dShell;
	int fileSize;
	char fName1[7];
	char fName2[7];
	char dirSect[512];

	interrupt(0x21, 2, dirSect, 2, 0);
	
	// Iterate on directory sector entries
	dShell = 0;
	while( dShell<512 ){

		// If it's a zero starting entry; continue to the next one.
		if( *(dirSect+dShell)==0x0 ) { dShell+=32; continue; }

		// Empty the filename char[]
		for(iShell=0; iShell<7; iShell++) fName1[iShell]=0x0;

		// Load the filename
		*(fName1+0)=*(dirSect+dShell+0); *(fName1+1)=*(dirSect+dShell+1); *(fName1+2)=*(dirSect+dShell+2);
		*(fName1+3)=*(dirSect+dShell+3); *(fName1+4)=*(dirSect+dShell+4); *(fName1+5)=*(dirSect+dShell+5);
		*(fName1+6)=0x0;
		
		// Print name
		interrupt(0x21, 0, fName1, 0, 0);
		interrupt(0x21, 0, " :: Size = \0", 0, 0);

		// Calculate filesize in sectors
		fileSize = 0; iShell=6;
		while( iShell<32 && *(dirSect+dShell+iShell)!=0x00 ) { fileSize++; iShell++; }

		// Parsing the Filesize to be printable as characters of a string; we used fName2[] for that purpose
		*fName2 = (DIV(fileSize, 10)) + 48;
		*(fName2+1) = (MOD(fileSize, 10)) + 48;
		*(fName2+2) = '\0';

		// Print filesize then continue to the next entry
		interrupt(0x21, 0, fName2, 0, 0);
		interrupt(0x21, 0, " Sectors. \n\r\0", 0, 0);

		dShell+=32;
	}
}

void doCopy(char* CMD){

	int iShell;
	int iShelltmp;
	int fileSize;
	char fName1[7];
	char fName2[7];

	// [fileName1] parsing
	iShell = 0;
	while( iShell<7 && *(CMD+5+iShell)!=0xD && *(CMD+5+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+5+iShell); iShell++; }
	iShelltmp = iShell;
	while( iShelltmp<7 ) { *(fName1+iShelltmp) = 0x00; iShelltmp++; }

	if( *(CMD+5+iShell)!=' ' ) 
		{ interrupt(0x21, 0, "COPY-ERROR 0xA5: Wrong CMD format, It should be \"copy fName1 fName2\". \0", 0, 0); return; }

	// [fileName2] parsing
	iShell++;
	iShelltmp = 0;
	while( iShelltmp<7 && *(CMD+5+iShell+iShelltmp) != 0xD && *(CMD+5+iShell+iShelltmp) != ' ' )
		{ *(fName2+iShelltmp) = *(CMD+5+iShell+iShelltmp); iShelltmp++; }
	
	if( iShelltmp==0 ) { 
		interrupt(0x21, 0, "COPY-ERROR 0xA5: Wrong CMD format, It should be \"copy fName1 fName2\". \0", 0, 0); return;
	}

	while( iShelltmp<7 ) { *(fName2+iShelltmp) = 0x00; iShelltmp++; }

	
	// Read [fileName1]
	interrupt(0x21, 3, fName1, shellBuf, 0);

	// Calculate No. of sectors occupied by [fileName1] in iShell to be used for write interrupt of [fileName2]
	for(iShell=0; iShell<26; iShell++){
		for(iShelltmp=0; iShelltmp<512; iShelltmp++){
			if( *(shellBuf+(iShell*512)+iShelltmp) != 0x0 ) break;
		}
		if(iShelltmp==512) break;
	}

	if( iShell==0 ) {
		interrupt(0x21, 0, "COPY-ERROR 0xA5: The file you want to copy does not exist. \0", 0, 0); return;
	}

	// Write [fileName2] using Interrupt 8
	interrupt(0x21, 8, fName2, shellBuf, iShell);

	interrupt(0x21, 0, "File: \"\0", 0, 0);
	interrupt(0x21, 0, fName1, 0, 0);
	interrupt(0x21, 0, "\" copied successfully to file \"\0", 0, 0);
	interrupt(0x21, 0, fName2, 0, 0);
	interrupt(0x21, 0, "\".\0", 0, 0);


}
void doCreate(char* CMD){

	int iShell;
	int iShelltmp;
	int secNum;
	char fName1[7];
	int i=0;
	
	// [fileName1] parsing.
	iShell=0;
	while( iShell<7 && *(CMD+7+iShell)!=0xD && *(CMD+7+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+7+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }
	iShell=0;
	for(i=0;i<4;i++){
		
		// Empty the line read buffer
		for(iShelltmp=0; iShelltmp<80; iShelltmp++) *(readBuf+iShelltmp) = 0x0;
		
		// Wait for a line
		interrupt(0x21, 0, ">\0", 0, 0);
		interrupt(0x21, 1, readBuf, 0, 0);
		
		// If he pressed enter with no entered data; break;
		if( *readBuf == 0xD || readBuf =="asd") {
			      
			break;
		}
		
		// Else: copy the line to a buffer to be written to [fileName1] at the end;
		// then continue ask for a new line.
		iShelltmp=0;
		while( *(readBuf+iShelltmp) != 0x0 ){
			
			*(shellBuf+iShell) = *(readBuf+iShelltmp);
			iShell++; iShelltmp++;
		}
		
	}

	// Calculate no. of sectors needed and Write to disk
	secNum = DIV(iShell, 512) + 1;
	interrupt(0x21, 8, fName1, shellBuf, secNum);

	interrupt(0x21, 0, "File written successfully to disk. \0", 0, 0);

}


int cmdNumber(char* CMD){

	if( *(CMD)== 't' && *(CMD+1)== 'y' && *(CMD+2)== 'p' && *(CMD+3)== 'e' && *(CMD+4)== ' ' ){
		return 0;		// A:>type [fileName1]
	}

	if( *(CMD)== 'e' && *(CMD+1)== 'x' && *(CMD+2)== 'e' && *(CMD+3)== 'c' && *(CMD+4)== 'u'
						&& *(CMD+5)== 't' && *(CMD+6)== 'e' && *(CMD+7)== ' ' ){
		return 1;		// A:>execute [fileName1]
	}

	if( *(CMD)== 'd' && *(CMD+1)== 'e' && *(CMD+2)== 'l' && *(CMD+3)== 'e' && *(CMD+4)== 't'
						&& *(CMD+5)== 'e' && *(CMD+6)== ' ' ){
		return 2;		// A:>delete [fileName1]
	}

	if( *(CMD)== 'd' && *(CMD+1)== 'i' && *(CMD+2)== 'r' && *(CMD+3)== 0xD ){
		return 3;		// A:>dir
	}

	if( *(CMD)== 'c' && *(CMD+1)== 'o' && *(CMD+2)== 'p' && *(CMD+3)== 'y' && *(CMD+4)== ' ' ){
		return 4;		// A:>copy [fileName1] [fileName2]
	}

	if( *(CMD)== 'c' && *(CMD+1)== 'r' && *(CMD+2)== 'e' && *(CMD+3)== 'a' && *(CMD+4)== 't'
						&& *(CMD+5)== 'e' && *(CMD+6)== ' ' ){
		return 5;		// A:>create [fileName1]
	}
	if(*(CMD)== 'k' && (CMD+1)=='i' && *(CMD+2)=='l' && *(CMD+3)=='l' && *(CMD+4)==' '){
		return 6;
	}

	return -1;

}
void doExecute(char* CMD){

	int iShell;
	char fName1[7];

	// Pasring [fileName1]
	iShell = 0;
	while( iShell<7 && *(CMD+8+iShell)!=0xD && *(CMD+8+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+8+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }

	// Execute using Interrupt 4
	interrupt(0x21, 4, fName1, 0x2000, 0);

	/* At normal the shell will be returned to memory through the terminate() call
	 * Which will cause that this next statement be unreachable; However in case of 
	 * No file with the [fileName1] the executeProgram() function will return before 
	 * loading the program to memory; Hence this next print interrupt will be executed
	 * Notifying the user that the program he asked for does not exist
	 */
	interrupt(0x21, 0, "EXECUTE-ERROR 0xA2: program not found.\0", 0, 0);

}
void doView(char* CMD){
	
	int iShell;
	char fName1[7];
	
	// Parsing [fileName1]
	iShell = 0;
	while( iShell<7 && *(CMD+5+iShell)!=0xD && *(CMD+5+iShell)!=' ' ) { *(fName1+iShell) = *(CMD+5+iShell); iShell++; }
	while( iShell<7 ) { *(fName1+iShell) = 0x00; iShell++; }

	// Try to read; if the buffer is full print, else print an error.
	interrupt(0x21, 3, fName1, shellBuf, 0);
	if( *shellBuf==0 ) interrupt(0x21, 0, "VIEW-ERROR 0xA1: Either file not exists or you've not specified a name. \0", 0, 0); 
	else interrupt(0x21, 0, shellBuf, 0, 0);

}

void clear(char* buff, int len)
        {
	int i;
	for(i=0;i<len;i++)
        {
		buff[i] = 0x0;
	}
}
int MOD(int x, int y){
	if(x<y) return x;
	else return MOD(x-y, y);
}

int DIV(int x, int y){
	if(x<y) return 0;
	else return (1+DIV(x-y,y));
}



