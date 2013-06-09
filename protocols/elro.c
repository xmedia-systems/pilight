/*
	Copyright (C) 2013 CurlyMo

	This file is part of the Raspberry Pi 433.92Mhz transceiver.

    Raspberry Pi 433.92Mhz transceiver is free software: you can redistribute
	it and/or modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of the License,
	or (at your option) any later version.

    Raspberry Pi 433.92Mhz transceiver is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Raspberry Pi 433.92Mhz transceiver. If not, see
	<http://www.gnu.org/licenses/>
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "elro.h"

void elroParseRaw() {
}

void elroParseCode() {
}

char *elroParseBinary() {
	char *message = malloc((50*sizeof(char))+1);
	memset(message,'0',sizeof(message));

	int unit = binToDec(elro.binary,0,4);
	int state = elro.binary[10];
	int check = elro.binary[11];
	int id = binToDec(elro.binary,5,9);

	if(check != state) {
		sprintf(message,"id %d unit %d state",id,unit);
		if(state==1)
			strcat(message," on");
		else
			strcat(message," off");
		return message;
	} else {
		return (0);
	}
}

void elroCreateLow(int s, int e) {
	int i;

	for(i=s;i<=e;i+=4) {
		elro.raw[i]=(PULSE_LENGTH);
		elro.raw[i+1]=(elro.pulse*PULSE_LENGTH);
		elro.raw[i+2]=(elro.pulse*PULSE_LENGTH);
		elro.raw[i+3]=(PULSE_LENGTH);
	}
}

void elroCreateHigh(int s, int e) {
	int i;

	for(i=s;i<=e;i+=4) {
		elro.raw[i]=(PULSE_LENGTH);
		elro.raw[i+1]=(elro.pulse*PULSE_LENGTH);

		elro.raw[i+2]=(PULSE_LENGTH);
		elro.raw[i+3]=(elro.pulse*PULSE_LENGTH);
	}
}
void elroClearCode() {
	elroCreateLow(0,49);
}

void elroCreateUnit(int unit) {
	int binary[255];
	int length = 0;
	int i=0, x=0;

	length = decToBinRev(unit, binary);
	for(i=0;i<=length;i++) {
		if(binary[i]==1) {
			x=(i+1)*4;
			elroCreateHigh(1+(x-3),1+x);
		}
	}
}

void elroCreateId(int id) {
	int binary[255];
	int length = 0;
	int i=0, x=0;

	length = decToBinRev(id, binary);
	for(i=0;i<=length;i++) {
		if(binary[i]==1) {
			x=(i+1)*4;
			elroCreateHigh(21+(x-3),21+x);
		}
	}
}

void elroCreateState(int state) {
	if(state == 0) {
		elroCreateHigh(44,47);
	} else {
		elroCreateHigh(40,43);
	}
}

void elroCreateFooter() {
	elro.raw[48]=(PULSE_LENGTH);
	elro.raw[49]=(elro.footer*PULSE_LENGTH);
}

void elroCreateCode(struct options *options) {
	int id = -1;
	int unit = -1;
	int state = -1;

	if(atoi(getOption(options,'i')) > 0)
		id=atoi(getOption(options,'i'));
	if(atoi(getOption(options,'f')) == 1)
		state=0;
	else if(atoi(getOption(options,'t')) == 1)
		state=1;
	if(atoi(getOption(options,'u')) > -1)
		unit = atoi(getOption(options,'u'));

	if(id == -1 || unit == -1 || state == -1) {
		fprintf(stderr, "elro: insufficient number of arguments\n");
	} else {
		elroClearCode();
		elroCreateUnit(unit);
		elroCreateId(id);
		elroCreateState(state);
		elroCreateFooter(state);
	}
}

void elroPrintHelp() {
	printf("\t -t --on\t\t\tsend an on signal\n");
	printf("\t -t --off\t\t\tsend an off signal\n");
	printf("\t -u --unit=unit\t\t\tcontrol a device with this unit code\n");
	printf("\t -i --id=id\t\t\tcontrol a device with this id\n");
}

void elroInit() {

	strcpy(elro.id,"elro");
	strcpy(elro.desc,"Elro Switches");
	elro.header = 4;
	elro.pulse = 4;
	elro.footer = 45;
	elro.multiplier[0] = 0.1;
	elro.multiplier[1] = 0.3;
	elro.rawLength = 50;
	elro.binaryLength = 12;
	elro.repeats = 2;

	elro.bit = 0;
	elro.recording = 0;

	struct option elroOptions[] = {
		{"id", required_argument, NULL, 'i'},
		{"unit", required_argument, NULL, 'u'},
		{"on", no_argument, NULL, 't'},
		{"off", no_argument, NULL, 'f'},
		{0,0,0,0}
	};

	elro.options=setOptions(elroOptions);
	elro.parseRaw=&elroParseRaw;
	elro.parseCode=&elroParseCode;
	elro.parseBinary=elroParseBinary;
	elro.createCode=&elroCreateCode;
	elro.printHelp=&elroPrintHelp;

	protocol_register(&elro);
}
