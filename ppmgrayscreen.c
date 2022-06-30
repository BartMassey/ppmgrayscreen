/*
 * Copyright © Bart Massey Feb 2004
 * ALL RIGHTS RESERVED
 * [This program is licensed under the "3-clause ('new') BSD License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */

/*
 * ppmgrayscreen.c - select gray areas, or specific colored areas
 * Loosely based on
 * ppmcolormask.c - produce PBM mask of areas containing certain color
 * written by Bryan Henderson in 2000
 */

#define _DEFAULT_SOURCE
    /* This makes sure strdup() is in string.h */

#include <string.h>
#include "ppm.h"
#include "pbm.h"
#include "shhopt.h"

static struct cmdline_info {
    /* All the information the user supplied in the command line,
       in a form easy for the program to use.
    */
    char *input_filename;
    pixel mask_color;
    double tolerance;
    int verbose;
    char *mask_string;
    char *tolerance_string;
} cmdline;

static void
parse_command_line(int argc, char ** argv,
                   struct cmdline_info *cmdline_p);



void hsv(pixval *rgb, double *hue, double *sat, double *val) {
    pixval min = rgb[0];
    pixval max = rgb[0];
    pixval delta;
    int i;
    for (i = 1; i < 3; i++) {
	if (rgb[i] < min)
	    min = rgb[i];
	if (rgb[i] > max)
	    max = rgb[i];
    }
    *val = max / (double) PPM_MAXMAXVAL;
    *sat = -1;
    *hue = -1;
    if (max == 0)
	return;
    delta = max - min;
    *sat = delta / (double) PPM_MAXMAXVAL;
    if (delta == 0)
	return;
    if (rgb[0] == max)
	*hue = (rgb[1] - rgb[2]) / (6.0 * delta);
    else if (rgb[1] == max)
	*hue = 1/3.0 + (rgb[2] - rgb[0]) / (6.0 * delta);
    else if (rgb[2] == max)
	*hue = 2/3.0 + (rgb[0] - rgb[1]) / (6.0 * delta);
    if (*hue < 0)
	*hue += 1;
}

static double absf(double f) {
    if (f >= 0)
	return f;
    return -f;
}

static double anglediff(double a1, double a2) {
    double da = absf(a1 - a2);
    if (da <= 0.5)
	return da;
    return 1.0 - da;
}

static int
miscolored(pixel *target, double mh, double ms, double mv) {
    pixval t[3] = {target->r, target->g, target->b};
    double h, s, v;
    hsv(t, &h, &s, &v);
    if (v > mv)
	return 1;
    if (v == 0)
	return 0;
    if (s <= 0)
	return 0;
    if (mh < 0)
	return s >= cmdline.tolerance;
    return (anglediff(h, mh) > cmdline.tolerance);
}

int
main(int argc, char *argv[]) {

    FILE* ifp;

    /* Parameters of input image: */
    int rows, cols;
    pixval maxval;
    int format;
    pixel ppm_white;

    PPM_ASSIGN(ppm_white, PPM_MAXMAXVAL, PPM_MAXMAXVAL, PPM_MAXMAXVAL);

    ppm_init(&argc, argv);

    parse_command_line(argc, argv, &cmdline);

    if (cmdline.input_filename != NULL) 
        ifp = pm_openr(cmdline.input_filename);
    else
        ifp = stdin;

    ppm_readppminit(ifp, &cols, &rows, &maxval, &format);
    ppm_writeppminit(stdout, cols, rows, maxval, 0);
    {
        pixel* const row = ppm_allocrow(cols);
	pixval m[3] = { cmdline.mask_color.r,
			cmdline.mask_color.g,
			cmdline.mask_color.b };
	double h, s, v;
        int r;
	hsv(m, &h, &s, &v);
	for (r = 0; r < rows; ++r) {
	    int col;
	    ppm_readppmrow(ifp, row, cols, maxval, format);
	    for (col = 0; col < cols; ++col)
		if (miscolored(&row[col], h, s, v))
		    row[col] = ppm_white;
	    ppm_writeppmrow(stdout, row, cols, maxval, 0);
	}
        ppm_freerow(row);
    }

    pm_close(ifp);

    exit(0);
    }



static void
parse_command_line(int argc, char ** argv,
                   struct cmdline_info *cmdline_p) {
/*----------------------------------------------------------------------------
   Note that many of the strings that this function returns in the
   *cmdline_p structure are actually in the supplied argv array.  And
   sometimes, one of these strings is actually just a suffix of an entry
   in argv!
-----------------------------------------------------------------------------*/
    optStruct *option_def = malloc(100*sizeof(optStruct));
        /* Instructions to OptParseOptions on how to parse our options.
         */
    unsigned int option_def_index;

    PPM_ASSIGN(cmdline_p->mask_color, 0xff, 0xff, 0xff);
    cmdline_p->tolerance = 0.1;

    option_def_index = 0;   /* incremented by OPTENTRY */
    OPTENTRY('v', "verbose",    OPT_FLAG,   &cmdline_p->verbose,        0);
    OPTENTRY('m', "mask",    OPT_STRING,   &cmdline_p->mask_string,        0);
    OPTENTRY('t', "tolerance",    OPT_STRING,   &cmdline_p->tolerance_string,        0);

    /* Set the defaults */
    cmdline_p->verbose = 0;

    pm_optParseOptions(&argc, argv, option_def, 0);
        /* Uses and sets argc, argv, and all of *cmdline_p. */

    if (cmdline_p->mask_string)
	cmdline_p->mask_color =
	  ppm_parsecolor(cmdline_p->mask_string, PPM_MAXMAXVAL);

    if (cmdline_p->tolerance_string)
	cmdline_p->tolerance = atof(cmdline_p->tolerance_string);

    if (argc == 1)
        cmdline_p->input_filename = NULL;  /* he wants stdin */
    else if (argc == 2) {
        if (strcmp(argv[1], "-") == 0)
            cmdline_p->input_filename = NULL;  /* he wants stdin */
        else 
            cmdline_p->input_filename = strdup(argv[1]);
    } else 
        pm_error("usage");

}

