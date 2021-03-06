/*
 * output.c -- create netcdf file for heat equation simulation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "output.h"
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>

extern int	debug;

/**
 * \brief create the output file, generic version
 */
static heatfile_t	*create0(int dim, const char *filename,
	double h, double ht, int nx, int ny) {
	heatfile_t	*hf = (heatfile_t *)malloc(sizeof(heatfile_t));
	if (NULL == hf) {
		fprintf(stderr, "cannot allocate output file structure\n");
		return NULL;
	}
	hf->dim = dim;
	hf->nx = nx;
	hf->ny = ny;
	hf->ncid = -1;
	if (debug) {
		fprintf(stderr, "%s:%d: creating NetCDF file %s\n",
			__FILE__, __LINE__, filename);
	}

	int	status;

	// create the file
	status = nc_create(filename, NC_NOCLOBBER, &hf->ncid);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot create file %s: %s\n", filename,
			nc_strerror(status));
	}

	// add variables hx, hy, and n
	int	hid, htid, nxid, nyid;
	status = nc_def_var(hf->ncid, "h", NC_DOUBLE, 0, NULL, &hid);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot create h variable: %s\n",
			nc_strerror(status));
		goto bad;
	}
	status = nc_def_var(hf->ncid, "ht", NC_DOUBLE, 0, NULL, &htid);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot create ht variable: %s\n",
			nc_strerror(status));
		goto bad;
	}
	status = nc_def_var(hf->ncid, "nx", NC_INT, 0, NULL, &nxid);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot create nx variable: %s\n",
			nc_strerror(status));
		goto bad;
	}
	if (dim == 2) {
		status = nc_def_var(hf->ncid, "ny", NC_INT, 0, NULL, &nyid);
		if (NC_NOERR != status) {
			fprintf(stderr, "cannot create ny variable: %s\n",
				nc_strerror(status));
			goto bad;
		}
	}

	// create the array dimensions
	size_t	lenx = nx;
	int	x_dim, y_dim, t_dim;
	if (NC_NOERR != (status = nc_def_dim(hf->ncid, "x", lenx, &x_dim))) {
		fprintf(stderr, "cannot define x dimension: %s\n",
			nc_strerror(status));
		goto bad;
	}
	if (dim == 2) {
		size_t	leny = ny;
		if (NC_NOERR != (status = nc_def_dim(hf->ncid, "y", leny,
			&y_dim))) {
			fprintf(stderr, "cannot define y dimension: %s\n",
				nc_strerror(status));
			goto bad;
		}
	}
	if (NC_NOERR != (status = nc_def_dim(hf->ncid, "t", NC_UNLIMITED,
		&t_dim))) {
		fprintf(stderr, "cannot define t dimension: %s\n",
			nc_strerror(status));
		goto bad;
	}

	// define the array
	int	dimensions[3] = { t_dim, x_dim, y_dim };
	if (NC_NOERR != (status = nc_def_var(hf->ncid, "u", NC_DOUBLE,
		dim + 1, dimensions, &hf->arrayid))) {
		fprintf(stderr, "cannot define u array: %s\n",
			nc_strerror(status));
		goto bad;
	}

	// end define mode
	if (NC_NOERR != (status = nc_enddef(hf->ncid))) {
		fprintf(stderr, "cannot end define mode: %s\n",
			nc_strerror(status));
		goto bad;
	}

	// add values for hx, ht and n
	status = nc_put_var_double(hf->ncid, hid, &h);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot write the h value: %s\n",
			nc_strerror(status));
		goto bad;
	}
	status = nc_put_var_double(hf->ncid, htid, &ht);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot write the ht value: %s\n",
			nc_strerror(status));
		goto bad;
	}
	status = nc_put_var_int(hf->ncid, nxid, &nx);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot write the nx value: %s\n",
			nc_strerror(status));
		goto bad;
	}
	if (dim == 2) {
		status = nc_put_var_int(hf->ncid, nyid, &ny);
		if (NC_NOERR != status) {
			fprintf(stderr, "cannot write the ny value: %s\n",
				nc_strerror(status));
			goto bad;
		}
	}

	return hf;
bad:
	if (hf->ncid >= 0) {
		nc_close(hf->ncid);
	}
	free(hf);
	return NULL;
}

/**
 * \brief create output file for 1-dim wave equation
 */
heatfile_t	*output_create(const char *filename, double h, double ht,
			int n) {
	return create0(1, filename, h, ht, n, n);
}

/**
 * \brief create output file for 2-dim wave equation
 */
heatfile_t	*output2_create(const char *filename, double h, double ht,
			int nx, int ny) {
fprintf(stderr, "create file %s\n", filename);
	return create0(2, filename, h, ht, nx, ny);
}

/**
 * \brief add a row to the result fiel
 */
int	output_add(heatfile_t *hf, int t, double *u) {
	size_t	start[2] = { t, 0 };
	size_t	size[2] = { 1, hf->nx };
	int	status = nc_put_vara(hf->ncid, hf->arrayid, start, size, u);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot write data: %s\n",
			nc_strerror(status));
	}
	return 0;
}

/**
 * \brief add a row to the result fiel
 */
int	output2_add(heatfile_t *hf, int t, double *u) {
	size_t	start[3] = { t, 0, 0 };
	size_t	size[3] = { 1, hf->nx, hf->ny };
	int	status = nc_put_vara(hf->ncid, hf->arrayid, start, size, u);
	if (NC_NOERR != status) {
		fprintf(stderr, "cannot write data: %s\n",
			nc_strerror(status));
	}
	return 0;
}

/**
 * \brief close the output file
 */
int	output_close(heatfile_t *hf) {
	int	status;
	if (NC_NOERR != (status = nc_close(hf->ncid))) {
		fprintf(stderr, "cannot close file: %s\n", nc_strerror(status));
		return -1;
	}
	free(hf);
	return 0;
}
