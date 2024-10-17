#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlinear.h>

typedef double (*opt_function)(void*, double, void*);

extern double rust_callback(opt_function, const gsl_vector*, size_t, double, double*, size_t);

struct data {
  size_t params_len;
  size_t vars_len;
  double* ts;
  double* ys;
  opt_function func_f;
  double* args;
  size_t args_len;
};

int
call_f (const gsl_vector * x, void *data,
        gsl_vector * f)
{
  size_t params_len = ((struct data *)data)->params_len;
  size_t n = ((struct data *)data)->vars_len;
  double *t = ((struct data *)data)->ts;
  double *y = ((struct data *)data)->ys;

  opt_function func_f = ((struct data *)data)->func_f;
  double *args = ((struct data *)data)->args;
  size_t args_len = ((struct data *)data)->args_len;

  size_t i;

  for (i = 0; i < n; i++)
    {
      /* Model Yi = A * exp(-lambda * t_i) + b */
      double Yi = rust_callback(func_f, x, params_len, t[i], args, args_len); 
      gsl_vector_set (f, i, Yi - y[i]);
    }

  return GSL_SUCCESS;
}


void
callback(const size_t iter, void *params,
         const gsl_multifit_nlinear_workspace *w)
{
  gsl_vector *f = gsl_multifit_nlinear_residual(w);
  gsl_vector *x = gsl_multifit_nlinear_position(w);
  double rcond;

  /* compute reciprocal condition number of J(x) */
  gsl_multifit_nlinear_rcond(&rcond, w);

  fprintf(stderr, "iter %2zu: A = %.4f, lambda = %.4f, b = %.4f, cond(J) = %8.4f\n",
          iter,
          gsl_vector_get(x, 0),
          gsl_vector_get(x, 1),
          gsl_vector_get(x, 2),
          1.0 / rcond
  );
}

void run_gsl_multifit_nlinear(
    double* params,
    size_t params_len,
    double* ts,
    double* ys,
    size_t vars_len,
    void* func_f,
    double* args,
    size_t args_len,
    size_t max_iters
) {

    const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
    gsl_multifit_nlinear_workspace *w;
    gsl_multifit_nlinear_fdf fdf;
    gsl_multifit_nlinear_parameters fdf_params = gsl_multifit_nlinear_default_parameters();

    gsl_vector *f;
    gsl_matrix *jacobian;
    gsl_matrix *covar = gsl_matrix_alloc (params_len, params_len);
  
    struct data d = {
       params_len,
       vars_len,
       ts,
       ys,
       func_f,
       args,
       args_len
    };

    gsl_vector_view x = gsl_vector_view_array (params, params_len);
    double chisq, chisq0;
    int status, info;

    const double xtol = 1e-8;
    const double gtol = 1e-8;
    const double ftol = 1e-8;

    fdf.f = call_f;
    fdf.df = NULL;
    fdf.fvv = NULL;
    fdf.n = vars_len;
    fdf.p = params_len;
    fdf.params = &d;

    /* allocate workspace with default parameters */
    w = gsl_multifit_nlinear_alloc (T, &fdf_params, vars_len, params_len);

    /* initialize solver with starting point and weights */
    gsl_multifit_nlinear_init (&x.vector, &fdf, w);

    /* compute initial cost function */
    f = gsl_multifit_nlinear_residual(w);
    gsl_blas_ddot(f, f, &chisq0);

    /* solve the system with a maximum of 100 iterations */
    status = gsl_multifit_nlinear_driver(max_iters, xtol, gtol, ftol, callback, NULL, &info, w);

    /* compute covariance of best fit parameters */
    jacobian = gsl_multifit_nlinear_jac(w);
    gsl_multifit_nlinear_covar (jacobian, 0.0, covar);

    /* compute final cost */
    gsl_blas_ddot(f, f, &chisq);

    gsl_multifit_nlinear_free (w);
    gsl_matrix_free (covar);
}
