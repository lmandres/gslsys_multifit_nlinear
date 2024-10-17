mod multifit_nlinear; 

use rand::random;


#[no_mangle]
fn expb_f(params: Vec<f64>, t: f64, _args: Vec<f64>) -> f64 {

    let a = params.get(0).unwrap();
    let lambda = params.get(1).unwrap();
    let b = params.get(2).unwrap();

    a * f64::exp(-lambda * t) + b
}

fn main() {

    let mut params = Vec::new();
    let mut ts = Vec::new();
    let mut ys = Vec::new();
    let args = Vec::new();

    params.push(1.0);
    params.push(1.0);
    params.push(0.0);

    for i in 0..100 {

        let rand_flt = random::<f64>();

        let ti = (i as f64) * 3.0 / (100.0 - 1.0);
        let yi = 1.0 + 5.0 * f64::exp(-1.5 * ti);
        let si = 0.1 * yi;
        let dy = si * rand_flt;

        ts.push(ti);
        ys.push(yi + dy);
    }

    unsafe {
        multifit_nlinear::gsl_multifit_nlinear_basic(params, ts, ys, expb_f, args);
    }
}
