#define _CRT_SECURE_NO_WARNINGS

// Force 8-byte struct packing to match libjpeg's expectations
#pragma pack(push, 8)

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <setjmp.h>

struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr* my_error_ptr;

void my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    printf("JPEG Error: %s\n", buffer);
    longjmp(myerr->setjmp_buffer, 1);
}

#pragma pack(pop)

int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.jpg output.jpg\n", argv[0]);
        return 1;
    }

    FILE* infile = fopen(argv[1], "rb");
    if (!infile) {
        fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
        return 1;
    }

    FILE* outfile = fopen(argv[2], "wb");
    if (!outfile) {
        fprintf(stderr, "Cannot open output file: %s\n", argv[2]);
        fclose(infile);
        return 1;
    }

    printf("Files opened successfully\n");

    // Decompression
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        fclose(outfile);
        return 1;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);

    printf("Reading JPEG header...\n");
    fflush(stdout);

    (void)jpeg_read_header(&cinfo, TRUE);

    printf("Header read successfully!\n");
    printf("Image: %d x %d, %d components\n",
        cinfo.image_width, cinfo.image_height, cinfo.num_components);

    (void)jpeg_start_decompress(&cinfo);

    int row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    // Compression
    struct jpeg_compress_struct dstinfo;
    struct my_error_mgr jerr2;

    dstinfo.err = jpeg_std_error(&jerr2.pub);
    jerr2.pub.error_exit = my_error_exit;

    if (setjmp(jerr2.setjmp_buffer)) {
        jpeg_destroy_compress(&dstinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        fclose(outfile);
        return 1;
    }

    jpeg_create_compress(&dstinfo);
    jpeg_stdio_dest(&dstinfo, outfile);

    dstinfo.image_width = cinfo.output_width;
    dstinfo.image_height = cinfo.output_height;
    dstinfo.input_components = cinfo.output_components;
    dstinfo.in_color_space = cinfo.out_color_space;

    jpeg_set_defaults(&dstinfo);
    jpeg_set_quality(&dstinfo, 95, TRUE);

    printf("Starting compression...\n");
    jpeg_start_compress(&dstinfo, TRUE);

    printf("Processing scanlines...\n");
    while (cinfo.output_scanline < cinfo.output_height) {
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
        (void)jpeg_write_scanlines(&dstinfo, buffer, 1);
    }

    printf("Finishing...\n");
    jpeg_finish_compress(&dstinfo);
    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_compress(&dstinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(outfile);
    fclose(infile);

    printf("\n=== SUCCESS ===\n");
    printf("Metadata removed: %s\n", argv[2]);

    return 0;
}