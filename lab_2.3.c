#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

typedef struct  
{  
    unsigned short blue;  
    unsigned short green;  
    unsigned short red;  
    unsigned short reserved;  
}__attribute__((packed)) PIXEL; 


int main()
{
//---------------------------------------------------------------------------------------
// map I/O
//---------------------------------------------------------------------------------------
    int 		fbfd = 0;
    struct 		fb_var_screeninfo vinfo;
    struct 		fb_fix_screeninfo finfo;
    long int	screensize = 0;
    char 		*fbp = 0;
    int 		x = 0, y = 0;
    long int 	location = 0;

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) 
	{
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) 
	{
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) 
	{
        perror("Error reading variable information");
        exit(3);
    }
    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) 
	{
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");
//---------------------------------------------------------------------------------------
//show bmp file
//---------------------------------------------------------------------------------------
    FILE          	*fp_s = NULL;    	// source file handler
	FILE          	*fp_t = NULL;    	// target file handler
	unsigned int  	width, height;   	// image width, image height
	unsigned char 	*image_s = NULL;    // source image array
	unsigned char header[54] =
	{
		0x42,        // identity : B
		0x4d,        // identity : M
		0, 0, 0, 0,  // file size
		0, 0,        // reserved1
		0, 0,        // reserved2
		54, 0, 0, 0, // RGB data offset
		40, 0, 0, 0, // struct BITMAPINFOHEADER size
		0, 0, 0, 0,  // bmp width
		0, 0, 0, 0,  // bmp height
		1, 0,        // planes
		24, 0,       // bit per pixel
		0, 0, 0, 0,  // compression
		0, 0, 0, 0,  // data size
		0, 0, 0, 0,  // h resolution
		0, 0, 0, 0,  // v resolution
		0, 0, 0, 0,  // used colors
		0, 0, 0, 0   // important colors
	};

	unsigned int 	file_size;    			// file size
	unsigned int 	rgb_raw_data_offset; 	// RGB raw data offset
	unsigned short 	bit_per_pixel; 			// bit per pixel
	unsigned short 	byte_per_pixel;    		// byte per pixel
	const 	 char*	fname_s = "temp.bmp";
	unsigned short	R, G, B;
	 
	 
	
	fp_s = fopen(fname_s, "rb");
	if (fp_s == NULL)
	{
		printf("fopen fp_s error\n");
		return -1;
	}

	// move offset to 10 to find rgb raw data offset
	fseek(fp_s, 10, SEEK_SET);
	fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s);

	// move offset to 18    to get width & height;
	fseek(fp_s, 18, SEEK_SET);
	fread(&width,  sizeof(unsigned int), 1, fp_s);
	fread(&height, sizeof(unsigned int), 1, fp_s);

	// get  bit per pixel
	fseek(fp_s, 28, SEEK_SET);
	fread(&bit_per_pixel, sizeof(unsigned short), 1, fp_s);
	byte_per_pixel = bit_per_pixel / 8;

	// move offset to rgb_raw_data_offset to get RGB raw data
	fseek(fp_s, rgb_raw_data_offset, SEEK_SET);

	image_s = (unsigned char *)malloc((size_t)width * height * byte_per_pixel);
	if (image_s == NULL)
	{
		printf("malloc images_s error\n");
		return -1;
	}

	fread(image_s, sizeof(unsigned char), (size_t)(long)width * height * byte_per_pixel, fp_s);
	printf("%d\n",bit_per_pixel);
	
	// vertical inverse algorithm
	int y_avg = 0 + (height-1);
	for(y = 0; y != height; ++y)
	{
		for(x = 0; x != width; ++x)
		{
			location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;
			
		    int y_temp = y_avg - y;
			R = *(image_s + byte_per_pixel * (width * y_temp + x) + 2);
			G = *(image_s + byte_per_pixel * (width * y_temp + x) + 1);
			B = *(image_s + byte_per_pixel * (width * y_temp + x) + 0);

			*(fbp + location + 0) = B;
			*(fbp + location + 1) = G;
			*(fbp + location + 2) = R;
			*(fbp + location + 3) = 0;
		}
	}

	fclose(fp_s);
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
 }