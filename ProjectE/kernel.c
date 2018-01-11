void printString(char * string);
void readString(char * string);
void readSector(char * buffer, int sector);
void handleInterrupt21(int ax, int bx, int cx, int dx);
void readFile(char * filename);
void writeFile(char * name, char * buffer, int numberOfSectors);
void executeProgram(char * name);
void terminate();
void printDirectory();
void handleTimerInterrupt(int segment, int sp);
void killProcess(int pid);
void executeForegroundProgram(char * name);

int mod(int a, int b);
int div(int a, int b);

typedef struct {
	int active;
	int sp;
	int waiting[8];
} process;


process processes[8];
int currentProcess = 0;

int main(void) {
	int i;
	int j;
	for(i = 0; i < 8; i++) {
		for(j = 0; j < 8; j++) {
			processes[i].waiting[j] = 0;
		}

		processes[i].active = 0;
		processes[i].sp = 0xff00;
	}

	makeInterrupt21();
	makeTimerInterrupt();
	interrupt(0x21, 4, "shell\0", 0x2000, 0);
	while(1){}
	return 0;
}

void killProcess(char * rawString) {
	int pid = rawString[0] - 48;
	int i;
	if(pid>7 || pid<1){
		return;
	}

	setKernelDataSegment();
	processes[pid].active = 0;
	processes[0].waiting[1] = 0;
	restoreDataSegment();
}

void handleTimerInterrupt(int segment, int sp) {
	int i, j, k;
	int numberOfCheckedProcesses = 0;
	int waiting;
	
	if(segment == 0x1000 * (currentProcess+2))
		processes[currentProcess].sp = sp;

	for(i = mod(currentProcess + 1, 8); ; i = mod(i + 1, 8)) {
		waiting = 0;
		for(k = 0; k < 8; k++){
			if(processes[i].waiting[k] == 1) {
				waiting = 1;
				break;
			}
		}

		if(processes[i].active == 1 && waiting == 0) {
			currentProcess = i;
			returnFromTimer((i+2)*0x1000, processes[i].sp);
			return;
		}
		numberOfCheckedProcesses++;
		if(numberOfCheckedProcesses == 8)
			break;
	}
	returnFromTimer(segment, sp);
}


void executeProgram(char * name) {
	int i, k;
	char buffer[13312];
	int active, sp;
	readFile(name, buffer);
	if(buffer[0] == '\0')
		return;
	
	for(i = 0; i < 8; i++) {
		setKernelDataSegment();
		active = processes[i].active;
		sp = processes[i].sp;
		restoreDataSegment();
		if(active == 0) {
			setKernelDataSegment();
			processes[i].sp = 0xff00;	
			restoreDataSegment();
			for(k = 0; k < 13312; k++) 
				putInMemory((i + 2) * 0x1000, k, buffer[k]);
			initializeProgram((i + 2) * 0x1000);
			
			setKernelDataSegment();
			processes[i].active = 1;
			restoreDataSegment();
			return;
		}
	}
}



void readFile(char * filename, char * file_buffer) {
	int i;
	int j;
	int k;
	char buffer[512];
	char error_msg[17];

	error_msg[0] = 'F';
	error_msg[1] = 'i';
	error_msg[2] = 'l';
	error_msg[3] = 'e';
	error_msg[4] = ' ';
	error_msg[5] = 'n';
	error_msg[6] = 'o';
	error_msg[7] = 't';
	error_msg[8] = ' ';
	error_msg[9] = 'f';
	error_msg[10] = 'o';
	error_msg[11] = 'u';
	error_msg[12] = 'n';
	error_msg[13] = 'd';
	error_msg[14] = '\n';
	error_msg[15] = '\r';
	error_msg[16] = '\0';

	readSector(buffer, 2);

	for(i = 0; i < 512; i += 32) {
		for(j = 0; j<6; j++) {
			if(filename[j] != buffer[i + j]) 
				break;				
			
			if(j == 5) {
				for(k = 0; buffer[i+6+k] != 0x0; k++) {
					readSector(file_buffer+512*k, buffer[i + 6 + k]);
				}
				return;
			}
		}
	}



	printString(error_msg);

	for(i = 0; i < 13312; i++)
		file_buffer[i] = '\0';
}

void printString(char * string){
	char al;
	char ah = 0xe;
	int ax;
	int i;

	for(i = 0; string[i] != '\0'; i++){
 		al = string[i];
	 	ax = ah * 256 + al;
		interrupt(0x10, ax, 0, 0, 0);
	}
}

void readString(char * string){
	char c;
	int i = 0;
	char ah = 0xe;
	while(i < 77){
 		c = interrupt(0x16, 0, 0, 0, 0);
		/* if enter */
		if(c == 0xd) {
			break;
		} 
		/* if backspace */
		else if(c == 0x8) {
			if(i > 0) {
				/* print the backspace, print a whitespace and print the backspace again to remove the character */
				interrupt(0x10, ah * 256 + c, 0, 0, 0);
				interrupt(0x10, ah * 256 + ' ', 0, 0, 0);
				interrupt(0x10, ah * 256 + c, 0, 0, 0);
				i--;
			}
		} 
		/* if normal character */
		else {
			string[i] = c;
			interrupt(0x10, ah * 256 + c, 0, 0, 0);
			i++;
		}
	}

	interrupt(0x10, ah * 256 + 0xa, 0, 0, 0);
	interrupt(0x10, ah * 256 + '\r', 0, 0, 0);
	string[i] = 0xa;
	string[i+1] = '\r';
	string[i+2] = 0x0;
}

void readSector(char * buffer, int sector) {
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

void printDirectory(){
	char directory[512];
	char buffer[80];
	int sectorCount;
	int i;
	int bufferIndex;
	int j;
	int k;
	int sectorOnesDigit;

	readSector(directory, 2);

	buffer[0] = '\n';
	buffer[1] = '\r';
	buffer[2] = '\0';
	printString(buffer);

	buffer[0]= 'C';
	buffer[1]= 'u';
	buffer[2]= 'r';
	buffer[3]= 'r';
	buffer[4]= 'e';
	buffer[5]= 'n';
	buffer[6]= 't';
	buffer[7]= ' ';
	buffer[8]= 'F';
	buffer[9]= 'i';
	buffer[10]= 'l';
	buffer[11]= 'e';
	buffer[12]= 's';
	buffer[13]= ':';
	buffer[14]= '\n';
	buffer[15]= '\n';
	buffer[16]= '\r';
	buffer[17]= '\0';

	printString(&buffer);

	for(i = 0; i < 512; i += 32) {
		if(!(directory[i] == 0x0)){
			sectorCount=0;
			bufferIndex = 0;
			buffer[bufferIndex] =' ';
			buffer[bufferIndex+1] =' ';
			bufferIndex+=2;
			
			for(j = 0; directory[i+j]!=0x0 && j < 6; j++){
				buffer[bufferIndex] = directory[i+j];
				bufferIndex++;
			}
			
			for(; j < 6; j++){
				buffer[bufferIndex] =' ';
				bufferIndex++;
			}

			for(k = 0; k < 26; k++){
				if(directory[i+6+k]==0){
					break;
				}
				sectorCount++;			
			}

			buffer[bufferIndex] =' ';
			buffer[bufferIndex+1] =':';
			buffer[bufferIndex+2] =' ';
			bufferIndex+=3;

			if(sectorCount>9){
				buffer[bufferIndex] = ((sectorCount/10)+48);
				bufferIndex++;
			}

			sectorOnesDigit = sectorCount;
			while(sectorOnesDigit > 9){
				sectorOnesDigit -=10;
			}
			buffer[bufferIndex] = ((sectorOnesDigit)+48);
			bufferIndex++;
			
			buffer[bufferIndex] = ' ';
			buffer[bufferIndex+1] = 's';
			buffer[bufferIndex+2] = 'e';
			buffer[bufferIndex+3] = 'c';
			buffer[bufferIndex+4] = 't';
			buffer[bufferIndex+5] = 'o';
			buffer[bufferIndex+6] = 'r';
			bufferIndex+=7;

			if(sectorCount>1){
				buffer[bufferIndex] = 's';
				bufferIndex++;
			}

			buffer[bufferIndex] =0xa;
			buffer[bufferIndex+1] = '\r';
			buffer[bufferIndex+2] ='\0';
			printString(&buffer);
		}
	}
	buffer[0] = '\n';
	buffer[1] = '\r';
	buffer[2] = '\0';
	printString(buffer);
}

void writeSector(char * buffer, int sector) {
	int ah = 3;
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

void writeFile(char* name, char* buffer, int numberOfSectors){

	char map[512];
	char directory[512];
	int i, j, k;
	int nameLength;
	int foundDirOpening = 0;
	int freeSectors[26];
	int freeSectorCount=0;
	int mapIndex=0;
	char error_msg[26];
	char success_msg[15];

	error_msg[0] = 'I';
	error_msg[1] = 'n';
	error_msg[2] = 's';
	error_msg[3] = 'u';
	error_msg[4] = 'f';
	error_msg[5] = 'f';
	error_msg[6] = 'i';
	error_msg[7] = 'c';
	error_msg[8] = 'i';
	error_msg[9] = 'e';
	error_msg[10] = 'n';
	error_msg[11] = 't';
	error_msg[12] = ' ';
	error_msg[13] = 'f';
	error_msg[14] = 'r';
	error_msg[15] = 'e';
	error_msg[16] = 'e';
	error_msg[17] = ' ';
	error_msg[18] = 's';
	error_msg[19] = 'p';
	error_msg[20] = 'a';
	error_msg[21] = 'c';
	error_msg[22] = 'e';
	error_msg[23] = '\n';
	error_msg[24] = '\r';
	error_msg[25] = '\0';

	success_msg[0] = 'F';
	success_msg[1] = 'i';
	success_msg[2] = 'l';
	success_msg[3] = 'e';
	success_msg[4] = ' ';
	success_msg[5] = 'c';
	success_msg[6] = 'r';
	success_msg[7] = 'e';
	success_msg[8] = 'a';
	success_msg[9] = 't';
	success_msg[10] = 'e';
	success_msg[11] = 'd';
	success_msg[12] = '\n';
	success_msg[13] = '\r';
	success_msg[14] = '\0';
	
	readSector(map, 1);
	readSector(directory, 2);

	while(1){
		if(freeSectorCount == numberOfSectors){
			break;
		} 
		if(mapIndex == 512){
			printString(error_msg);
			return;
		} 
		if(map[mapIndex] == 0x0){
			freeSectors[freeSectorCount]=mapIndex;
			freeSectorCount++;
		}
		mapIndex++;
	}


	for(nameLength = 0; name[nameLength]!= '\0'; nameLength++);


	for(i = 0; i < 512; i += 32) {
		if(directory[i] == 0x0){
			foundDirOpening = 1;
			for(j = 0; j < nameLength && j < 6; j++){
				directory[i + j] =name[j];
			}
			for(; j < 6; j++){
				directory[i + j] =0x0;
			}
			break;
		}
	}

	if(!foundDirOpening){
		printString(error_msg);
		return;
	}

	for(k = 0; k < numberOfSectors; k++){
		map[freeSectors[k]]=0xFF;
		directory[i+6+k] = freeSectors[k];
		writeSector(&(buffer[512*k]), freeSectors[k]);
	}

	for(; k < 26; k++){
		directory[i + 6+ k] =0x0;
	}

	writeSector(map, 1);
	writeSector(directory, 2);	
	printString(success_msg);	 

}

void deleteFile(char * filename){
	char map[512];
	char directory[512];
	int i;
	int j;
	int k;
	char error_msg[17];
	char success_msg[15];

	error_msg[0] = 'F';
	error_msg[1] = 'i';
	error_msg[2] = 'l';
	error_msg[3] = 'e';
	error_msg[4] = ' ';
	error_msg[5] = 'n';
	error_msg[6] = 'o';
	error_msg[7] = 't';
	error_msg[8] = ' ';
	error_msg[9] = 'f';
	error_msg[10] = 'o';
	error_msg[11] = 'u';
	error_msg[12] = 'n';
	error_msg[13] = 'd';
	error_msg[14] = '\n';
	error_msg[15] = '\r';
	error_msg[16] = '\0';

	success_msg[0] = 'F';
	success_msg[1] = 'i';
	success_msg[2] = 'l';
	success_msg[3] = 'e';
	success_msg[4] = ' ';
	success_msg[5] = 'd';
	success_msg[6] = 'e';
	success_msg[7] = 'l';
	success_msg[8] = 'e';
	success_msg[9] = 't';
	success_msg[10] = 'e';
	success_msg[11] = 'd';
	success_msg[12] = '\n';
	success_msg[13] = '\r';
	success_msg[14] = '\0';

	readSector(map,1);
	readSector(directory, 2);

	for(i = 0; i < 512; i += 32) {
		for(j = 0; j<6; j++) {
			if(filename[j] != directory[i + j]) 
				break;				
			
			if(j == 5) {
				for(k = 0; directory[i+6+k] != 0x0 && k< 26; k++) {
					int sectorToDelete = directory[i + 6 + k];
					map[sectorToDelete] = 0x0;
				}
				directory[i] = 0x0;
				writeSector(map, 1);
				writeSector(directory, 2);			
			
				printString(success_msg);
				return;
			}
		}
	}

	printString(error_msg);

}

void terminate() {
	int i;
	setKernelDataSegment();
	for(i = 0; i < 8; i++){
		processes[i].waiting[currentProcess]=0;
	}
	processes[currentProcess].active = 0;
	processes[0].waiting[1] = 0;
	while(1) {
	}
}

void handleInterrupt21(int ax, int bx, int cx, int dx) {
	if(ax == 0) 
		printString(bx);
	else if(ax == 1) 
		readString(bx);
	else if(ax == 2) 
		readSector(bx, cx);
	else if(ax == 3)
		readFile(bx, cx);
	else if(ax == 4) 
		executeProgram(bx);
	else if(ax == 5) 
		terminate();
	else if(ax == 6)
		writeSector(bx, cx);
	else if(ax == 7)
		deleteFile(bx);
	else if(ax == 8)
	  writeFile(bx, cx, dx);
	else if(ax == 9)
	  printDirectory();
	else if(ax == 10) 
		killProcess(bx);
	else if(ax == 11) { 
		setKernelDataSegment();
		processes[0].waiting[1] = 1;
		restoreDataSegment();
		executeProgram(bx);
	}
	else if(ax > 11)
		printString("Error in handleInterrupt21. ax is too big\0");
}

int mod(int a, int b) {
	while(a >= b)  
		a -= b;
	return a;
}

int div(int a, int b) {
	int quotient = 0;
	while((quotient + 1) * b <= a)
		quotient += 1;
	return quotient;
}
