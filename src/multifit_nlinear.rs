#[allow(improper_ctypes)]
#[link(name = "gslmfnlin")]
extern "C" {
    fn run_gsl_multifit_nlinear(
        params: *const f64,
        covars: *const f64,
        params_len: usize,
        ts: *const f64,
        ys: *const f64,
        vars_len: usize,
        func_f: fn(Vec<f64>, f64, Vec<f64>) -> f64,
        args: *const f64,
        args_len: usize,
        max_iters: u64
    );
}

pub unsafe fn gsl_multifit_nlinear_basic(
    params_in: Vec<f64>,
    ts: Vec<f64>,
    ys: Vec<f64>,
    func_f: fn(Vec<f64>, f64, Vec<f64>) -> f64,
    args: Vec<f64>,
    max_iters: u64
) -> (Vec<f64>, Vec<f64>) {

    if ts.len() != ys.len() {
        eprintln!("Time length does not match Ys length!");
        return (vec![], vec![]);
    }

    let mut params: Vec<f64> = params_in.clone();
    let mut covars: Vec<f64> = Vec::with_capacity(params_in.len());

    covars.resize(params_in.len(), 0.0);

    unsafe {
        run_gsl_multifit_nlinear(
            params.as_mut_ptr(),
            covars.as_mut_ptr(),
            params.len(),
            ts.as_ptr(),
            ys.as_ptr(),
            ts.len(),
            func_f,
            args.as_ptr(),
            args.len(),
            max_iters
        );
    }

    (params, covars)
}
