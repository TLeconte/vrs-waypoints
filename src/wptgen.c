/*
 *  Copyright (c) 2018 Thierry Leconte
 *
 *   
 *   This code is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void usage(void)
{
	printf("wptgen fix.dat and nav.dat to jscript converter for vrs-waypoints plugin\nCopyright (c) 2018 Thierry Leconte  \n\n");
	printf("usage : wptgen [-f fixfile] [-n navfile ] -- lat lon [sizelat sizelon]\n\n");
	printf ("convert fix.dat and nav.dat fix to an waypoints.html file useable by vrs-waypoints plugin\n");
	printf ("take only waypoints that are on a square delimited by lat-sizelat lon+sizelon\n");
	printf ("by default slat and slon = 2.0. Don't use too large value if you don't want a too big output file\n");
	printf ("lat,lon,sizelat,sizelon in decimal degree, negativ for West and South\n");
	printf("Options :\n");
	printf("\t-f fixfile : path of fix.dat file (default : fix.dat)\n");
	printf("\t-n navfile : path of nav.dat file (default : nav.dat)\n");
	printf("Example :\n");
	printf("wptgen -f data/fix.dat -n data/nav.dat -- 46.53 2.40\n\n");

}

static void outheader()
{
printf("<script type=\"text/javascript\">\n");
printf("waypoints = [");
}

static void outfooter()
{
printf("];\n</script>\n");
}

static int testzone(float lat,float lon,float clat,float clon,float slat,float slon)
{
if(lat>clat-slat && lat<clat+slat &&
   lon>clon-slon && lon<clon+slon ) return 1;
return 0;
}

static int nbp=0;

static int parsefix(char *filename,float clat,float clon,float slat,float slon)
{
	FILE *fd;
	char line[1024];

	fd=fopen(filename,"r");
	if(fd==NULL) return -1;

	if(fscanf(fd,"%1s\n",line)!=1)
		return -1;
	if(strncmp(line,"I",1)!=0)
		return -1;

	if(fscanf(fd,"%3s *\n",line)!=1)
		return -1;
	if(strncmp(line,"600",3)!=0)
		return -1;

	while(!feof(fd)) {
		int n;
		float lat,lon;
		char name[6];

		fgets(line,1024,fd);
		if(line[0]=='\n') continue;

		n=sscanf(line,"%f %f %5s\n",&lat,&lon,name);
		if(n!=3) continue;

		if(testzone(lat,lon,clat,clon,slat,slon)==0) continue;

		if(strspn(name,"ABCDEFGHIJKLMNOPQRSTUVWXYZ")!=5) continue;

		if(nbp!=0) printf(",");
		printf("{type: \"fix\",name: \"%s\",lat:%f,lng:%f}",name,lat,lon);
		nbp++;

	}
	fclose(fd);
	return 0;
}

static int parsenav(char *filename,float clat,float clon,float slat,float slon)
{
	FILE *fd;
	char line[1024];

	fd=fopen(filename,"r");
	if(fd==NULL) return -1;

	if(fscanf(fd,"%1s\n",line)!=1)
		return -1;
	if(strncmp(line,"I",1)!=0)
		return -1;

	if(fscanf(fd,"%3s *\n",line)!=1)
		return -1;
	if(strncmp(line,"810",3)!=0)
		return -1;

	while(!feof(fd)) {
		int n;
		int type,len;
		float lat,lon;
		int freq;
		char id[6];
		char *name,*ntype;

		fgets(line,1024,fd);
		strtok(line,"\n");
		strtok(line,"\r");
		if(line[0]==0) continue;

		n=sscanf(line,"%1d %f %f %*d %d %*d %*f %5s %n",&type,&lat,&lon,&freq,id,&len);
		if(n<3) continue;

		if(testzone(lat,lon,clat,clon,slat,slon)==0) continue;

		name=&(line[len]);
		len=strlen(name);

		for(n=len-1;name[n]!=' ' && n>0;n--); 
		if(n==0) continue;
		ntype=&(name[n+1]);
		name[n]=0;

		switch(type) {
		 case 2 :
			if(nbp!=0) printf(",");
			if(strcmp(ntype,"NDB")==0)
				printf("{type:\"ndb\",name:\"%s %d\\n%s\",lat:%f,lng:%f}",id,freq,name,lat,lon);
			if(strcmp(ntype,"NDB-DME")==0)
				printf("{type:\"ndbdme\",name:\"%s %d\\n%s\",lat:%f,lng:%f}",id,freq,name,lat,lon);
			break;
		 case 3 :
			if(nbp!=0) printf(",");
			if(strcmp(ntype,"VOR")==0)
				printf("{type:\"vor\",name:\"%s %3.2f\\n%s\",lat:%f,lng:%f}",id,(float)freq/100,name,lat,lon);
			if(strcmp(ntype,"VOR-DME")==0)
				printf("{type:\"vordme\",name:\"%s %3.2f\\n%s\",lat:%f,lng:%f}",id,(float)freq/100,name,lat,lon);
			if(strcmp(ntype,"VORTAC")==0)
				printf("{type:\"vortac\",name:\"%s %3.2f\\n%s\",lat:%f,lng:%f}",id,(float)freq/100,name,lat,lon);
			break;
		 case 4 :
		 case 5 :
			if(nbp!=0) printf(",");
			printf("{type:\"loc\",name:\"%s %3.2f\\n%s\",lat:%f,lng:%f}",id,(float)freq/100,name,lat,lon);
			break;
		 case 13 :
			if(strcmp(ntype,"DME")) continue;
			if(nbp!=0) printf(",");
			printf("{type:\"dme\",name:\"%s %3.2f\\n%s\",lat:%f,lng:%f}",id,(float)freq/100,name,lat,lon);
			break;
	 	 default :
			continue;
		}
		nbp++;

	}
	fclose(fd);
	return 0;
}

int main(int argc, char **argv)
{
	int c;
	char *fixfilename="fix.dat";
	char *navfilename="nat.dat";
	float clat,clon;
	float slat=2,slon=2;

	while ((c = getopt(argc, argv, "f:n:")) != EOF) {
		switch (c) {
		case 'f':
			fixfilename = optarg;
			break;
		case 'n':
			navfilename = optarg;
			break;
		default:
			usage();
			return 1;
		}
	}

	if(optind+2>argc) 
		usage();

	clat=atof(argv[optind]);optind++;
	clon=atof(argv[optind]);optind++;

	if(optind<argc) {
		slat=atof(argv[optind]);optind++;
	}
	if(optind<argc) {
		slon=atof(argv[optind]);optind++;
	}

	outheader();
	if(parsefix(fixfilename,clat,clon,slat,slon)) {
		exit(1);
	}
	if(parsenav(navfilename,clat,clon,slat,slon)) {
		exit(1);
	}
	outfooter();

	return 0;
}
