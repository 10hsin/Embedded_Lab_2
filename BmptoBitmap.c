#include <stdio.h>
#include <stdlib.h>

int bmptobitstream(const char *fname_s, const char *fname_t)
{
	FILE          	*fp_s = NULL;    	// source file handler
	FILE          	*fp_t = NULL;    	// target file handler
	unsigned int  	x,y;             	// for loop counter
	unsigned int  	width, height;   	// image width, image height
	unsigned char 	*image_s = NULL;    // source image array
	unsigned char 	R, G, B, Y;			// color of R, G, B
	unsigned long 	RGB=0;

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

	 unsigned int 		file_size;           				// file size
	 unsigned int 		rgb_raw_data_offset; 	// RGB raw data offset
	 unsigned short 	bit_per_pixel;     			// bit per pixel
	 unsigned short 	byte_per_pixel;    		// byte per pixel

	 fp_s = fopen(fname_s, "rb");
	 if (fp_s == NULL)
	 {
		printf("fopen fp_s error\n");
		return -1;
	 }

	 //write to .h file
	 fp_t = fopen(fname_t, "w");
	 if (fp_t == NULL)
	{
		printf("fopen fname_t error\n");
		return -1;
	}

	// write #define IMG_LENGTH
	fprintf(fp_t,"#define IMG_LENGTH ");

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

	//output IMG_LENGTH
	fprintf(fp_t,"%d\n#define IMG_WIDTH %d\n#define IMG_HEIGHT %d\nstatic u32 IMG_DATA[] = {\n",width*height,width, height);
	int y_avg = 0 + (height-1);

	for(y = 0; y != height; ++y)
	{
		for(x = 0; x != width; ++x)
		{
		    int y_temp = y_avg - y;
			R = *(image_s + byte_per_pixel * (width * y_temp + x) + 2);
			B = *(image_s + byte_per_pixel * (width * y_temp + x) + 1);
			G = *(image_s + byte_per_pixel * (width * y_temp + x) + 0);

			/*Y = (R+G+B) /3;
			R = Y;
			B = Y;
			G = Y;*/

			RGB = ((R&0xff)<<16) + ((G&0xff)<<8) + ((B&0xff));
			fprintf(fp_t, "0x00%x,\n", RGB);

		}
	}
  // write tail
  fprintf(fp_t,"};");

 fclose(fp_s);
 fclose(fp_t);

 return 0;
}

int main()
{
	bmptobitstream("temp.bmp", "cf_hdmi_demo.h");
}
