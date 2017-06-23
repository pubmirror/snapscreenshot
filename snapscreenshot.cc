#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

#include "argh.h"
#include "autoptr"

static unsigned scrncount = 6;  /* number of terminals to shoot */
static unsigned xtimes = 4;     /* number of them to place horizontally */
static unsigned ytimes;
static unsigned firstwin = 1;
static unsigned char bordercolor = 9;
static unsigned char fontwidth = 9;

const char ProgName[] = "snapscreenshot";
void PrintVersionInfo(void)
{
    printf(
        "%s version "VERSION"\n"
        "Copyright (C) 1992,2003 Bisqwit (http://iki.fi/bisqwit/)\n", ProgName);
}
void PrintDescription(void)
{
    printf("\nTakes snapshot from your Linux console(s) and outputs a Targa file."
           "\nYou can use convert(1) to convert the resulting image to various formats.\n");
}
struct Option Options[] =
{
    /* ID-----c--c2----txt--------descr---------------- */
    {'?',   'h',  0,"help",      "Help"},
    {'f',   'f',  0,"fontwidth", "Font width (8 or 9, default: 9)"},
    {'b',   'b',  0,"border",    "Border colour (0..15, default: 9)"},
    {'1',   '1',  0,"firstwin",  "Specify the index of first window"},
    {'c',   'c',  0,"num",       "Specify number of windows (default: 6)"},
    {'x',   'x',  0,"alongside", "Specify number of windows horizontally (4)"},
    {'v',   'V',  0,"version",   "Version information"},
    {0,0,0,NULL,NULL}
};
void DefArg(const char *arg)
{
	if(!strcmp(arg, "this"))
	{
		struct stat St;
		fstat(0, &St);
		scrncount = 1;
		xtimes = 1;
		firstwin = St.st_rdev & 255;
	}
	else
		ArgError(2, arg);
}
void Case(int ID, const char **dummy)
{
    switch(ID)
    {
        case 0:    /* startup */
            break;
        case 'b':
        	bordercolor = ParamRange(0, 15, dummy);
            break;
        case 'f':
        	fontwidth = ParamRange(8, 9, dummy);
            break;
        case 'c':
            scrncount = ParamRange(1, 256, dummy);
            break;
        case '1':
            firstwin = ParamRange(1, 24, dummy);
            break;
        case 'x':
            xtimes = ParamRange(1, 256, dummy);
            break;
        case 'v':
            PrintVersionInfo();
            exit(EXIT_SUCCESS);
        case '?': Heelp = 1; break;
        case -1: /* finish args, set default values if necessary */
        	if(xtimes > scrncount)
        	{
        		cerr << "Error: can't place horizontally more windows than there is.\n";
        		exit(EXIT_FAILURE);
        	}
        	ytimes = (scrncount + xtimes - 1) / xtimes;
        	cerr << "Geometry will be: " << xtimes << 'x' << ytimes << endl;
            break;
    }
}

class font : public ptrable
{
public:
	unsigned X, Y, size;
	vector<unsigned char> pixeldata;
	font(unsigned char fontwidth)
	{
		struct consolefontdesc desc;
		desc.charcount = 512;
		desc.charheight = 16;
		desc.chardata = new char[512*16];
		
		cerr << "Reading font...\n";
		
		int fd = open("/dev/tty2", O_RDONLY);
		if(fd < 0)
		{
			perror("/dev/console");
			exit(EXIT_FAILURE);
		}
		if(ioctl(fd, GIO_FONTX, &desc) < 0)
		{
			perror("GIO_FONTX");
			exit(EXIT_FAILURE);
		}
		close(fd);
		
		pixeldata.resize(size = desc.charcount * (X = fontwidth) * (Y = desc.charheight));
		
		unsigned q=0, p=0;
		for(unsigned a=0; a<desc.charcount; ++a)
		{
			for(unsigned y = 0; y < Y; ++y)
			{
				unsigned char c = desc.chardata[q++];
				unsigned x;
				for(x = 0; x < 8; ++x)
					pixeldata[p++] = (c >> (7-x)) & 1;
				for(; x < X; ++x)pixeldata[p++] = 0;
			}
			q += (32-Y);
		}
	}
};

typedef autoptr<font> fontp;

class screen : public ptrable
{
public:
	unsigned X, Y, size;
	vector<unsigned short> textdata;
	vector<unsigned char> pixeldata;
	
	fontp font;
	
	static unsigned char flip(unsigned char c)
	{
		return (c & 10)
		     | (4*(c & 1))
		     | ((c & 4)/4);
	}
	screen(unsigned ttynum, fontp f) : font(f)
	{
		unsigned char Buf[64], b, c;
		unsigned a, x, y;
		
		sprintf((char *)Buf, "/dev/vcc/a%u", ttynum);
		int fd = open((char *)Buf, O_RDONLY);
		if(fd < 0)
		{
			sprintf((char *)Buf, "/dev/vcsa%u", ttynum);
			fd = open((char *)Buf, O_RDONLY);
			if(fd < 0)
			{
				perror((char *)Buf);
				fprintf(stderr,
				  "Perhaps such console does not exist?\n"
				  "Try: echo >/dev/vcsa%u\n"
				  "This should activate the console as a blank one.\n"
				  "You can edit /etc/inittab to make use for it.\n"
				  "If this is not what you want, then use the -c command line\n"
				  "switch to limit the number of consoles the program tries to read.\n",
				  ttynum
				);
				             
				exit(EXIT_FAILURE);
			}
		}
		cerr << "Analyzing " << Buf;
		
		if(read(fd, &b, 1) < 1
        || read(fd, &c, 1) < 1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
		Y = b;
		X = c;
		
		read(fd, &b, 1); /* 0x3E == ? */
		read(fd, &c, 1); /* 0x00 == ? */
		
		cerr << " (" << X << ", " << Y << ")\n";
		cerr << "Cursor: " << (unsigned)b << ", " << (unsigned)c << ")\n";
		
		textdata.resize(size = X*Y);
		
		read(fd, &textdata[0], size * 2);
		close(fd);
		
	#if 0
		X = 40;
		Y = 24;
	#endif
		
		size = X * Y * font->X * font->Y;
		
		pixeldata.resize(size);
		
		a=0;
		for(y=0; y<Y; ++y)
		{
			for(x=0; x<X; ++x)
			{
				setchar(x, y,
					textdata[a]&255,
				    flip((textdata[a]>>8) & 15),
				    flip((textdata[a]>>12) & 15));
				++a;
			}
	#if 0
			a += 120-X;
	#endif
		}
	}
	void setchar(unsigned x, unsigned y, unsigned char ch,
	             unsigned char fc, unsigned char bc)
	{
		unsigned charsize = font->Y * font->X;
		unsigned char *p = &font->pixeldata[charsize * ch];
		
		unsigned w = (y * X * font->Y + x) * font->X;
		
		for(unsigned yy = 0; yy < font->Y; ++yy)
		{
			for(unsigned xx = 0; xx < font->X; ++xx)
				pixeldata[w++] = *p++ ? fc : bc;
			w += (X-1) * font->X;
		}
	}
	unsigned xdim() const { return X * font->X; }
	unsigned ydim() const { return Y * font->Y; }
};

typedef autoptr<screen> screenp;

class palette
{
public:
	unsigned X, Y, size;
	unsigned char pal[768];
	palette()
	{
		cerr << "Reading palette...\n";
		
		int fd = open("/dev/console", O_RDONLY);
		ioctl(fd, GIO_CMAP, &pal);
		
		#if 0
		for(unsigned c=8; c<15; ++c)
		{
			for(unsigned a=0; a<3; ++a)
				pal[c*3+a] = pal[(63-15+c)*3+a];
		}
		#endif
		
		close(fd);
	}
	void output() const
	{
		unsigned c;
		for(c=0; c<16; ++c)
		{
			cout << (char) (pal[c*3+2]);
			cout << (char) (pal[c*3+1]);
			cout << (char) (pal[c*3+0]);
		}
	}
};

class targa
{
public:
	targa(unsigned xsize, unsigned ysize)
	{
		cerr << "Outputting!\n";
	
	    outbyte(0);    /* id field len */
	    outbyte(1);    /* color map type */
	    outbyte(1);    /* image type code  */
	    outword(0);    /* palette start */
	    outword(16);   /* palette size */
	    outbyte(24);   /* palette bitness */
	    outword(0); outword(0);
	    outword(xsize); outword(ysize);
	    outbyte(8);    /* pixel bitness */
	    outbyte(0x28); /* misc */
	}
	~targa()
	{
		cerr << "Closing down.\n";
	}
    void outword(unsigned c)
    {
        cout << (char)(c & 255);
        cout << (char)(c >> 8);
    }
    void outbyte(unsigned char c)
    {
        cout << (char)c;
    }
    void outline(unsigned count, unsigned char c)
    {
    	string s(count, c);
    	
    	cout.write(s.c_str(), count);
    }
    void operator << (const palette &p)
    {
        p.output();
    }
    inline void operator << (int c)
    {
    	outbyte(c);
    }
    void write(const unsigned char *c, unsigned count)
    {
    	cout.write((const char *)c, count);
    }
};

int main(int argc, const char *const *argv)
{
	ReadArgs(argc, argv, "> snap.tga", "");
	
	fontp fontp = new font(fontwidth);
	
	vector<screenp> scr(scrncount);
	palette palette;
	
	for(unsigned a=0; a<scrncount; ++a)
		scr[a] = new screen(a+firstwin, fontp);

	/* All screens have the same size anyway. */
	targa tga(scr[0]->xdim()*xtimes + (xtimes-1)*2,
	          scr[0]->ydim()*ytimes + (ytimes-1)*2);
	          
	tga << palette;
	
	for(unsigned ay = 0; ; )
	{
		for(unsigned pxy = 0; pxy < scr[0]->ydim(); ++pxy)
		{
			for(unsigned ax = 0; ; )
			{
				unsigned scrn = ay*xtimes + ax;
				if(scrn >= scrncount)
				{
					// Empty window (no terminal!)
					tga.outline(scr[0]->xdim(), 8);
				}
				else
				{
					unsigned xm = scr[scrn]->xdim();
					unsigned char *pxd = &scr[scrn]->pixeldata[pxy*xm];
					
					tga.write(pxd, xm);
				}
					
				if(++ax == xtimes)break;
				tga << bordercolor;
				tga << bordercolor;
			}
		}
		if(++ay == ytimes)break;
		for(unsigned pax=0; pax<2; ++pax)
    		for(unsigned ax=0; ; )
    		{
    			unsigned xm = scr[0]->xdim();
    			tga.outline(xm, bordercolor);
    			if(++ax == xtimes)break;
    			tga << bordercolor;
    			tga << bordercolor;
    		}
	}
	return 0;
}
